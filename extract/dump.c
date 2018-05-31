/* dump.h */

#include <stdint.h>
#include <stdio.h>

#include "dump.h"

static void dump_line(const uint8_t *data, uint32_t len, uint32_t addr)
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

void dump_mem(const void *data, size_t size)
{
  const uint8_t *p = data;
  uint32_t addr = 0;
  for (size_t i = 0; i < size; i += 16) {
    dump_line(p + i, size - i, addr);
    addr += 16;
  }
}

