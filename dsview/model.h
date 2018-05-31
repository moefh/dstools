/* model.h */

#ifndef MODEL_H_FILE
#define MODEL_H_FILE

#include <stdint.h>

struct model {
  uint32_t n_vtx;
  uint32_t n_tri;
  
  float *vtx;
  uint32_t *indices;
};

int load_model(struct model *model, const char *filename);

#endif /* MODEL_H_FILE */
