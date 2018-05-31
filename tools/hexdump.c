/* hexdump.c */

#include <stdio.h>
#include <stdint.h>

static void dump_line(uint8_t *data, uint32_t len, uint32_t addr)
{
  printf("%08x | ", addr);

  unsigned char str[16];
  if (len > 16)
    len = 16;
  for (uint32_t i = 0; i < len; i++) {
    printf("%02x ", data[i]);
    str[i] = (data[i] >= 32 && data[i] < 127) ? data[i] : '.';
  }
  for (uint32_t i = len; i < 16; i++)
    printf("   ");
  printf("| %.*s\n", len, str);
}

static void dump_file(FILE *f)
{
  uint32_t addr = 0;
  uint8_t buf[4096];
  size_t buf_len;

  while (1) {
    buf_len = fread(buf, 1, sizeof(buf), f);
    if (buf_len == 0)
      break;

    for (uint32_t i = 0; i < buf_len; i += 16) {
      dump_line(buf + i, buf_len - i, addr);
      addr += 16;
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("USAGE: hexdump file\n");
    return 1;
  }

  FILE *f = fopen(argv[1], "rb");
  if (! f) {
    printf("hexdump: can't open %s\n", argv[1]);
    return 1;
  }
  dump_file(f);
  fclose(f);
  return 0;
}
