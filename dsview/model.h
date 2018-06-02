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
int load_model_colors(const char *filename, void (*set_color)(int num, float *color), int max_colors);

#endif /* MODEL_H_FILE */
