/* binfloat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
  unsigned char data[256];
  int data_n = 0;
  
  for (int i = 1; i < argc; i++) {
    data[data_n++] = strtoul(argv[i], NULL, 16);
  }

  float f;
  for (int i = 0; i+4 <= data_n; i += 4) {
    memcpy(&f, data+i, 4);
    printf("%02x %02x %02x %02x -> %20.17f\n", data[i], data[i+1], data[i+2], data[i+3], f);
  }
  
  return 0;
}
