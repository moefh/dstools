/* util.c */

#include <stdlib.h>
#include <string.h>

#ifdef WIN32

#include <direct.h>
#include <sys/stat.h>  

static int create_dir(const char *path, unsigned int mode)
{
  (void)mode;
  return _mkdir(path);
}

static int dir_exists(const char *path)
{
  struct _stat st;
  if (_stat(path, &st) != 0)
    return 0;
  return (st.st_mode & S_IFMT) == S_IFDIR;
}

#else

#include <sys/stat.h>

#define create_dir mkdir

static int dir_exists(const char *path)
{
  struct stat st;
  if (stat(path, &st) != 0)
    return 0;
  return (st.st_mode & S_IFMT) == S_IFDIR;
}

#endif

#include "util.h"

static int mkdir_p_r(char *dir, unsigned int mode)
{
  char *slash = strrchr(dir, '/');
  if (slash) {
    *slash = '\0';
    int ret = mkdir_p_r(dir, mode);
    *slash = '/';
    if (ret != 0)
      return ret;
  }

  if (dir_exists(dir))
    return 0;
  return create_dir(dir, mode);
}

int mkdir_p(const char *dir, unsigned int mode)
{
  char *dir_buf = malloc(strlen(dir)+1);
  if (! dir_buf)
    return 1;
  strcpy(dir_buf, dir);

  int ret = mkdir_p_r(dir_buf, mode);
  free(dir_buf);
  return ret;
}

