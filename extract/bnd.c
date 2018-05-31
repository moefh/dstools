/* bnd.c */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "bnd.h"
#include "dcx.h"
#include "reader.h"

static uint32_t read_u32(struct BND_FILE *bnd, size_t offset)
{
  if (offset + 4 > bnd->size)
    return 0;
  
  if (bnd->big_endian)
    return get_u32_be(bnd->data, offset);
  else
    return get_u32_le(bnd->data, offset);
}

int bnd_open(struct BND_FILE *bnd, const char *filename)
{
  char magic[4];
  
  if (read_file_data(filename, 0, magic, 4) != 0)
    return 1;

  if (memcmp(magic, "BND3", 4) == 0) {
    bnd->data = read_file(filename, &bnd->size);
  } else if (memcmp(magic, "DCX", 4) == 0) {
    bnd->data = dcx_read_file(filename, &bnd->size);
  } else {
    return 1;
  }
  if (! bnd->data)
    return 1;

  bnd->big_endian = true;
  bnd->file_id_sequential = false;
  uint32_t flags = read_u32(bnd, 12);
  if (flags == 0x74000000 || flags == 0x54000000 || flags == 0x70000000)
    bnd->big_endian = false;

  switch (flags) {
  case 0x70000000: bnd->file_def_stride = 0x14; break;
  case 0x74000000: bnd->file_def_stride = 0x18; break;
  case 0x00010100: bnd->file_def_stride = 0x0c; bnd->file_id_sequential = true; break;
  case 0x0E010100: bnd->file_def_stride = 0x14; break;
  case 0x2E010100: bnd->file_def_stride = 0x18; break;
  default:
    goto err;
  }
  
  bnd->n_files = read_u32(bnd, 16);
  return 0;

 err:
  free(bnd->data);
  return 1;
}

void bnd_close(struct BND_FILE *f)
{
  free(f->data);
  f->data = NULL;
}

void *bnd_get_file(struct BND_FILE *bnd, unsigned int file_num, size_t *p_size, char **p_name)
{
  if (file_num >= bnd->n_files)
    return NULL;

  uint32_t file_size = read_u32(bnd, 0x24 + file_num*bnd->file_def_stride);
  uint32_t file_off = read_u32(bnd, 0x28 + file_num*bnd->file_def_stride);
  uint32_t name_off = read_u32(bnd, 0x30 + file_num*bnd->file_def_stride);

  if (p_size)
    *p_size = file_size;
  if (p_name)
    *p_name = (bnd->file_id_sequential) ? NULL : (char *) bnd->data + name_off;
  return bnd->data + file_off;
}
