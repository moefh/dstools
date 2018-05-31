/* dump_hkx.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bhd.h"
#include "dcx.h"
#include "reader.h"
#include "dump.h"

#if 0
static void dump_mat4(float *m)
{
  for (int i = 0; i < 16; i++) {
    printf("%f  ", m[i]);
    if (i % 4 == 3)
      printf("\n");
  }
  printf("\n");
}
#endif

int write_file_data(const char *filename, void *data, size_t size)
{
  char fname[256];
  const char *p = strrchr(filename, '\\');
  p = p ? p+1 : filename;
  sprintf(fname, "%s.dat", p);
    
  FILE *f = fopen(fname, "wb");
  if (! f)
    return 1;

  if (fwrite(data, 1, size, f) != size) {
    fclose(f);
    return 1;
  }

  fclose(f);
  return 0;
}

static void dump_hkx(const char *filename, void *data, size_t size)
{
  uint32_t off = 8;

  printf("=============================================================================\n");
  printf("== %s\n", filename);
  printf("=============================================================================\n");
  printf("\n");
  
  while (off < size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    //if (memcmp((char *) data + off + 4, "DATA", 4) == 0) {
      printf("\n");
      printf("%08x %.4s len=%u\n", off, (char *) data + off + 4, chunk_size - 8);
      dump_mem((char *) data + off + 8, chunk_size - 8);
      printf("\n");
      //write_file_data(filename, data + off + 8, chunk_size - 8);
    //} else {
    //  printf("%08x %.4s len=%u\n", off, (char *) data + off + 4, chunk_size - 8);
    //}      
    off += chunk_size;
  }
  if (off != size)
    printf("warning: off != size (0x%08x != 0x%08x)\n", off, (uint32_t) size);
  printf("\n\n");
  //exit(0);
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("USAGE: bhdtool file.hkxbhd\n");
    return 1;
  }
  char *bhd_file = argv[1];

  struct BHD_FILE f;
  if (bhd_open(&f, bhd_file) != 0) {
    printf("Can't open '%s'\n", bhd_file);
    return 1;
  }

  for (uint32_t file_num = 0; file_num < f.n_files; file_num++) {
    char *filename;
    size_t comp_size;
    char *comp_data = bhd_get_file(&f, file_num, &comp_size, &filename);

    size_t size;
    void *data = dcx_read_mem(comp_data, comp_size, &size);
    if (! data) {
      printf("Can't inflate '%s'\n", filename);
    } else {
      dump_hkx(filename, data, size);
      free(data);
    }
  }
  
  bhd_close(&f);
  return 0;
}
