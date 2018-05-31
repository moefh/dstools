#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint32_t read_u32_le(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;

  return (d[3] << 24 | d[2] << 16 | d[1] << 8 | d[0]);
}

static float read_f32(const void *p, size_t offset)
{
  unsigned char *d = (unsigned char *) p + offset;
  //uint8_t data[4] = { d[3], d[2], d[1], d[0] };
  uint8_t data[4] = { d[0], d[1], d[2], d[3] };
  
  float ret;
  memcpy(&ret, data, 4);
  return ret;
}

static void *read_file(const char *filename, size_t *p_size)
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

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("USAGE: %s filename.nvm\n", argv[0]);
    return 1;
  }

  size_t size;
  void *data = read_file(argv[1], &size);

  uint32_t vtx_num = read_u32_le(data, 4);
  uint32_t vtx_off = read_u32_le(data, 8);
  
  //uint32_t xxx_num = read_u32_le(data, 12);
  uint32_t xxx_off = read_u32_le(data, 16);
  //uint32_t xxx_end = read_u32_le(data, 20);

  for (uint32_t ind = 0; ind < vtx_num; ind++) {
    printf("%08x  |", vtx_off + ind*4*3);
    for (int i = 0; i < 3; i++)
      printf("  %6.2f", read_f32(data, vtx_off + ind*4*3 + 4*i));
    printf("  [%u (0x%x)]", ind, ind);
    printf("\n");
  }
  
  printf("========================\n");
  
  for (uint32_t ind = 0; xxx_off + ind*4*7 < 0x4d00; ind++) {
    printf("%08x  |", xxx_off + ind*4*7);
    for (int i = 0; i < 7; i++)
      printf("  %08x", read_u32_le(data, xxx_off + ind*4*7 + 4*i));
    printf("  [%u]", ind);
    printf("\n");
  }

  printf("========================\n");
  for (uint32_t ind = 0; 0x4d00 + ind*4*4 < 0x6200; ind++) {
    printf("%08x  |", 0x4d00 + ind*4*4);
    for (int i = 0; i < 4; i++)
      printf(" %6.2f", read_f32(data, 0x4d00 + ind*4*4 + 4*i));
    printf("  [%u]", ind);
    printf("\n");
  }

  return 0;
}
