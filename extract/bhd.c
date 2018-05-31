/* bhd.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "bhd.h"
#include "reader.h"

static char *get_bdt_filename(const char *bhd_filename)
{
  size_t len = strlen(bhd_filename);
  if (len < 3)
    return NULL;
  
  char *bdt_filename = malloc(len+1);
  if (! bdt_filename)
    return NULL;
  
  strcpy(bdt_filename, bhd_filename);
  strcpy(bdt_filename + len - 3, "bdt");
  return bdt_filename;
}

int bhd_open(struct BHD_FILE *f, const char *bhd_filename)
{
  f->bhd = NULL;
  f->bdt = NULL;

  char *bdt_filename = get_bdt_filename(bhd_filename);
  if (! bdt_filename)
    goto err;

  f->bhd = read_file(bhd_filename, &f->bhd_size);
  if (f->bhd == NULL || f->bhd_size < 32)
    goto err;
  if (memcmp(f->bhd, "BHF3", 4) != 0)
    goto err;

  f->bdt = read_file(bdt_filename, &f->bdt_size);
  if (f->bdt == NULL || f->bdt_size < 16)
    goto err;
  if (memcmp(f->bdt, "BDF3", 4) != 0)
    goto err;

  f->n_files = get_u32_le(f->bhd, 16);
  
  free(bdt_filename);
  return 0;

 err:
  free(bdt_filename);
  free(f->bhd);
  free(f->bdt);
  return 1;
}

void bhd_close(struct BHD_FILE *f)
{
  free(f->bhd);
  free(f->bdt);
  f->bhd = NULL;
  f->bdt = NULL;
}

void *bhd_get_file(struct BHD_FILE *f, uint32_t file_num, size_t *p_size, char **p_name)
{
  uint32_t off = 0x20 + file_num * 0x18;

  *p_size = get_u32_le(f->bhd, off + 4);
  size_t file_off = get_u32_le(f->bhd, off + 8);
  if (p_name)
    *p_name = (char *) f->bhd + get_u32_le(f->bhd, off + 16);

  return (char *) f->bdt + file_off;
}
