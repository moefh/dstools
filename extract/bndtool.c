/* bndtool.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bnd.h"
#include "dcx.h"
#include "reader.h"
#include "dump.h"
#include "util.h"

#define MODE_LIST    0
#define MODE_EXTRACT 1
#define MODE_DUMP    2

#define FLAG_INFLATE  (1<<0)

static int write_file(const char *filename, void *data, size_t data_size)
{
  FILE *f = fopen(filename, "wb");
  if (! f)
    return 1;

  if (fwrite(data, 1, data_size, f) != data_size) {
    fclose(f);
    return 1;
  }

  fclose(f);
  return 0;
}

static void extract_file(const char *in_filename, void *data, size_t size, int inflated)
{
  const char *colon = strchr(in_filename, ':');
  if (colon)
    in_filename = colon + 1;

  while (*in_filename == '\\')
    in_filename++;
  
  if (strstr(in_filename, "..") != NULL) {
    printf("Refusing to extract file containing '..' in name ('%s')\n", in_filename);
    return;
  }

  char *filename = malloc(strlen(in_filename) + 1);
  if (! filename) {
    printf("Out of memory to extract file '%s'\n", in_filename);
    return;
  }
  strcpy(filename, in_filename);

  // remove '.dcx' extension if file was inflated
  if (inflated) {
    char *dot = strrchr(filename, '.');
    if (dot && strcmp(dot, ".dcx") == 0) {
      *dot = '\0';
    }
  }

  // convert backslashes to slashes
  for (char *p = filename; *p != '\0'; p++) {
    if (*p == '\\')
      *p = '/';
  }
  
  printf("-> extracting '%s'\n", filename);

  // create directory
  char *slash = strrchr(filename, '/');
  if (slash) {
    *slash = '\0';
    if (mkdir_p(filename, 0777) != 0)
      printf("Can't create directory '%s'\n", filename);
    *slash = '/';
  }

  // write file
  if (write_file(filename, data, size) != 0) {
    printf("ERROR writing '%s'\n", filename);
  }
}

static void process_file(const char *filename, void *data, size_t size, int mode, int flags)
{
  int inflated = 0;
  size_t orig_size = size;
  if ((flags & FLAG_INFLATE) && size >= 0x40 && memcmp(data, "DCX", 4) == 0) {
    size_t u_size;
    void *u_data = dcx_read_mem(data, size, &u_size);
    if (! u_data) {
      printf("ERROR inflating '%s'\n", filename);
      return;
    }
    size = u_size;
    data = u_data;
    inflated = 1;
  }
  
  switch (mode) {
  case MODE_LIST:
    if (inflated)
      printf("%8lu / %-8lu %s\n", (unsigned long) orig_size, (unsigned long) size, filename);
    else
      printf("%8lu %s\n", (unsigned long) size, filename);
    break;
    
  case MODE_EXTRACT:
    extract_file(filename, data, size, inflated);
    break;
    
  case MODE_DUMP:
    printf("-> %s:\n", filename);
    dump_mem(data, size, 0);
    break;
  }

  if (inflated)
    free(data);
}

static int read_cmdline(int argc, char *argv[], int *p_flags)
{
  if (argc != 3) {
    printf("USAGE: bndtool commands file.bnd\n");
    printf("\n");
    printf("Extract and list the contents of bnd files (BND3 format).\n");
    printf("\n");
    printf("Use one of these commands:\n");
    printf("  l    list files\n");
    printf("  d    dump files (hexdump)\n");
    printf("  x    extract files\n");
    printf("\n");
    printf("Optional flags for commands:\n");
    printf("  i    inflate extracted or dumped files (if applicable)\n");
    exit(1);
  }

  int mode = -1;
  int flags = 0;
  for (char *p = argv[1]; *p != '\0'; p++) {
    switch (*p) {
    case 'x': mode = MODE_EXTRACT; break;
    case 'l': mode = MODE_LIST; break;
    case 'd': mode = MODE_DUMP; break;
    case 'i': flags |= FLAG_INFLATE; break;
    default:
      printf("Invalid command: '%c'\n", *p);
      exit(1);
    }
  }

  if (mode < 0) {
    printf("At least one of 'x' or 'l' is required\n");
    exit(1);
  }

  *p_flags = flags;
  return mode;
}

int main(int argc, char *argv[])
{
  int flags;
  int mode = read_cmdline(argc, argv, &flags);
  char *bnd_file = argv[2];

  struct BND_FILE f;
  if (bnd_open(&f, bnd_file) != 0) {
    printf("Can't open '%s'\n", bnd_file);
    return 1;
  }

  for (uint32_t file_num = 0; file_num < f.n_files; file_num++) {
    char *filename;
    size_t size;
    char *data = bnd_get_file(&f, file_num, &size, &filename);

    char filename_buf[256];
    if (! filename) {
      snprintf(filename_buf, sizeof(filename_buf), "%u.dat", (unsigned int) file_num);
      filename = filename_buf;
    }
    
    process_file(filename, data, size, mode, flags);
  }
  
  bnd_close(&f);
  return 0;
}
