/* bnd.h */

#ifndef BND_H_FILE
#define BND_H_FILE

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct BND_FILE {
  unsigned char *data;
  size_t size;
  bool big_endian;
  bool file_id_sequential;
  uint32_t n_files;
  uint32_t file_def_stride;
};

int bnd_open(struct BND_FILE *bnd, const char *filename);
void bnd_close(struct BND_FILE *bnd);
void *bnd_get_file(struct BND_FILE *bnd, unsigned int file_num, size_t *p_size, char **p_name);

#endif /* BND_H_FILE */
