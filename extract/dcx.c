/* dcx.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "reader.h"
#include "zlib.h"

static int deflate_stream(struct READER reader, void *out, size_t out_size)
{
  int ret;
  z_stream strm;
  unsigned char in_buf[16384];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK)
    return ret;

  strm.avail_out = out_size;
  strm.next_out = out;
  do {
    size_t size = reader.read(&reader.r, in_buf, sizeof(in_buf));
    if (size == SIZE_MAX) {
      (void) inflateEnd(&strm);
      printf("* ERROR reading file\n");
      return 1;
    }
    strm.avail_in = size;
    if (strm.avail_in == 0)
      break;
    strm.next_in = in_buf;

    ret = inflate(&strm, Z_NO_FLUSH);
    switch (ret) {
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      inflateEnd(&strm);
      printf("* ERROR: inflate returns %d\n", ret);
      return 1;
    }
    if (strm.avail_out == 0 && ret != Z_STREAM_END) {
      printf("* ERROR: decompressed size is too big\n");
      inflateEnd(&strm);
      return 1;
    }
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  return (ret == Z_STREAM_END) ? 0 : 1;
}

static void *dcx_read(struct READER reader, size_t *p_out_size)
{
  unsigned char header[64];
  if (reader.read(&reader.r, header, sizeof(header)) != sizeof(header)) {
    printf("* ERROR: can't read header\n");
    return NULL;
  }
  if (memcmp(header, "DCX", 3) != 0) {
    printf("* ERROR: bad file magic\n");
    return NULL;
  }

  if (memcmp(header + 0x28, "DFLT", 4) == 0) {
    uint32_t start_off = get_u32_be(header, 0x14) + 0x20;
    size_t data_size = get_u32_be(header, 0x1c);
    //uint32_t comp_size = get_u32_be(header, 0x20);
    if (reader.set_pos(&reader.r, start_off) != 0) {
      printf("* ERROR: can't seek to position %u\n", start_off);
      return NULL;
    }
    //printf("decompressing from offset %u (0x%x)\n", start_off, start_off);
    void *data = malloc(data_size);
    if (! data) {
      printf("* ERROR: out of memory\n");
      return NULL;
    }
    if (deflate_stream(reader, data, data_size) != 0) {
      printf("* ERROR decompressing\n");
      free(data);
      return NULL;
    }
    *p_out_size = data_size;
    return data;
  }

  if (memcmp(header + 0x28, "EDGE", 4) == 0) {
    printf("* ERROR: 'EDGE' format not yet supported\n");
    return NULL;
  }

  printf("* ERROR: unknown format: '%.4s'\n", header + 40);
  return NULL;
}

void *dcx_read_file(const char *filename, size_t *p_out_size)
{
  FILE *f = fopen(filename, "rb");
  if (! f) {
    printf("* ERROR: can't open '%s'\n", filename);
    return NULL;
  }

  struct READER file_reader;
  reader_from_file(&file_reader, f);
  void *data = dcx_read(file_reader, p_out_size);
  fclose(f);
  return data;
}

void *dcx_read_mem(const void *data, size_t size, size_t *p_out_size)
{
  struct READER mem_reader;
  reader_from_memory(&mem_reader, data, size);
  return dcx_read(mem_reader, p_out_size);
}
