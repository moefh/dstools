/* dcxtool.c */

#include <stdio.h>
#include <stddef.h>

#include "dcx.h"

static int write_file(const char *filename, void *data, size_t data_size)
{
  FILE *f = fopen(filename, "wb");
  if (! f)
    return 1;

  if (fwrite(data, 1, data_size, f) != data_size) {
    fclose(f);
    return 1;
  }

  fclose(f);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("USAGE: %s in.dcx out\n", argv[0]);
    return 1;
  }
  
  size_t data_size;
  void *data = dcx_read_file(argv[1], &data_size);
  if (data == NULL)
    return 1;
  
  if (write_file(argv[2], data, data_size) != 0) {
    printf("inflate_dcx: can't write '%s'\n", argv[2]);
    return 1;
  }
  return 0;
}
