/* bhd.h */

#ifndef BHD_H_FILE
#define BHD_H_FILE

#include <stddef.h>
#include <stdint.h>

struct BHD_FILE {
  void *bhd;
  size_t bhd_size;
  void *bdt;
  size_t bdt_size;

  uint32_t n_files;
};

int bhd_open(struct BHD_FILE *f, const char *bhd_filename);
void bhd_close(struct BHD_FILE *f);
void *bhd_get_file(struct BHD_FILE *f, uint32_t file_num, size_t *p_size, char **p_name);

#endif /* BHD_H_FILE */
