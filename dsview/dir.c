/* dir.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "dir.h"
#include "debug.h"

typedef int (*find_callback)(const char *filename, void *data);

#ifdef xxxWIN32

#include <windows.h>

static int find_files(const char *dir, const char *ext, find_callback callback_func, void *callback_data)
{
  size_t find_len = strlen(dir) + 3 + strlen(ext) + 1;
  char *find = malloc(find_len);
  if (! find)
    return -1;
  snprintf(find, find_len, "%s\\*%s", dir, ext);

  WIN32_FIND_DATAA data;
  HANDLE h = FindFirstFileA(find, &data);
  free(find);
  if (h == INVALID_HANDLE_VALUE)
    return 1;

  int ret = 0;
  do {
    ret = callback_func(data.cFileName, callback_data);
  } while (ret == 0 && FindNextFileA(h, &data) != 0);
  FindClose(h);
  
  return 0;
}

#else

#include <dirent.h>

static int find_files(const char *dir, const char *ext, find_callback callback_func, void *callback_data)
{
  DIR *d = opendir(dir);
  if (! d)
    return 1;

  size_t ext_len = strlen(ext);
  
  int ret = 0;
  while (ret == 0) {
    struct dirent *entry = readdir(d);
    if (! entry)
      break;
    size_t name_len = strlen(entry->d_name);
    if (name_len >= ext_len && memcmp(entry->d_name + name_len - ext_len, ext, ext_len) == 0)
      ret = callback_func(entry->d_name, callback_data);
  }

  closedir(d);
  return ret;
}

#endif

struct list_dir_info {
  const char *dir;
  char **files;
  size_t alloc_files;
  size_t n_files;
};

static int add_file(const char *filename, void *data)
{
  struct list_dir_info *info = data;

  if (info->n_files + 2 >= info->alloc_files) {
    size_t alloc_files = info->alloc_files + 32;
    char **files = realloc(info->files, alloc_files * sizeof(*files));
    if (! files)
      return 1;
    info->files = files;
    info->alloc_files = alloc_files;
  }

  size_t name_size = strlen(info->dir) + 1 + strlen(filename) + 1;
  char *name = malloc(name_size);
  if (! name)
    return 1;
  snprintf(name, name_size, "%s/%s", info->dir, filename);
  info->files[info->n_files++] = name;
  info->files[info->n_files] = NULL;
  return 0;
}

static int compare_filenames(const void *p1, const void *p2)
{
  const char *const *f1 = p1;
  const char *const *f2 = p2;
  return strcasecmp(*f1, *f2);
}

char **dir_list_files(const char *dir, const char *ext)
{
  struct list_dir_info info = {
    .dir = dir,
    .files = NULL,
    .alloc_files = 0,
    .n_files = 0,
  };

  if (find_files(dir, ext, add_file, &info) != 0) {
    dir_free_files(info.files);
    return NULL;
  }

  qsort(info.files, info.n_files, sizeof(*info.files), compare_filenames);
  return info.files;
}

void dir_free_files(char **files)
{
  if (! files)
    return;
  for (int i = 0; files[i] != NULL; i++)
    free(files[i]);
  free(files);
}

