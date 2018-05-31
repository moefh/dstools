/* reader.c */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "reader.h"

void *read_file(const char *filename, size_t *p_size)
{
  FILE *f = fopen(filename, "rb");
  if (! f)
    return NULL;

  void *data = NULL;
  
  if (fseek(f, 0, SEEK_END) != 0)
    goto err;

  long size = ftell(f);
  if (size < 0)
    goto err;

  if (fseek(f, 0, SEEK_SET) != 0)
    goto err;

  data = malloc(size);
  if (! data)
    goto err;

  if (fread(data, 1, size, f) != (size_t) size)
    goto err;

  fclose(f);
  *p_size = size;
  return data;
  
 err:
  fclose(f);
  free(data);
  return NULL;
}

int read_file_data(const char *filename, size_t off, void *data, size_t size)
{
  FILE *f = fopen(filename, "rb");
  if (! f)
    return 1;

  if (fseek(f, off, SEEK_SET) != 0)
    goto err;

  if (fread(data, 1, size, f) != size)
    goto err;

  fclose(f);
  return 0;
  
 err:
  fclose(f);
  return 1;
}

// memory

static size_t mem_read(union READER_DATA *reader, void *data, size_t size)
{
  struct READER_MEM_DATA *mem = &reader->mem;

  if (size > mem->size - mem->pos)
    size = mem->size - mem->pos;
  memcpy(data, mem->data + mem->pos, size);
  mem->pos += size;
  return size;
}

static int mem_set_pos(union READER_DATA *reader, size_t pos)
{
  struct READER_MEM_DATA *mem = &reader->mem;

  if (mem->size < pos)
    return 1;
  mem->pos = pos;
  return 0;
}

void reader_from_memory(struct READER *r, const void *data, size_t size)
{
  r->r.mem.data = data;
  r->r.mem.size = size;
  r->r.mem.pos = 0;

  r->read = mem_read;
  r->set_pos = mem_set_pos;
}

// file

static size_t file_read(union READER_DATA *reader, void *data, size_t size)
{
  size_t ret = fread(data, 1, size, reader->f);
  if (ferror(reader->f))
    return SIZE_MAX;
  return ret;
}

static int file_set_pos(union READER_DATA *reader, size_t pos)
{
  return fseek(reader->f, pos, SEEK_SET);
}

void reader_from_file(struct READER *r, FILE *f)
{
  r->r.f = f;

  r->read = file_read;
  r->set_pos = file_set_pos;
}
