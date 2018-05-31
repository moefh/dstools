/* reader.h */

#ifndef READER_H_FILE
#define READER_H_FILE

#include <stdio.h>
#include <string.h>

struct READER_MEM_DATA {
  const void *data;
  size_t pos;
  size_t size;
};

union READER_DATA {
  struct READER_MEM_DATA mem;
  FILE *f;
};

struct READER {
  size_t (*read)(union READER_DATA *r, void *data, size_t size);
  int (*set_pos)(union READER_DATA *r, size_t pos);
  union READER_DATA r;
};

void *read_file(const char *filename, size_t *p_size);
int read_file_data(const char *filename, size_t off, void *data, size_t size);

void reader_from_file(struct READER *r, FILE *f);
void reader_from_memory(struct READER *r, const void *data, size_t size);


static inline uint32_t get_u32_be(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;
  return (d[0] << 24 | d[1] << 16 | d[2] << 8 | d[3]);
}

static inline uint16_t get_u16_be(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;
  return (d[0] << 8 | d[1]);
}

static inline uint32_t get_u32_le(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;
  return (d[3] << 24 | d[2] << 16 | d[1] << 8 | d[0]);
}

static inline uint16_t get_u16_le(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;
  return (d[1] << 8 | d[0]);
}

static inline float get_f32(const void *p, size_t offset)
{
  float ret;
  memcpy(&ret, (char *) p + offset, 4);
  return ret;
}

#endif /* READER_H_FILE */
