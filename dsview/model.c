/* model.c */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "model.h"
#include "debug.h"

static int read_u32_le(FILE *f, uint32_t *v)
{
  unsigned char b[4];

  if (fread(b, 1, 4, f) != 4)
    return 1;
  *v = b[0] | (b[1] << 8) | (b[2] << 16) | b[3] << 24;
  return 0;
}

int load_model(struct model *model, const char *filename)
{
  FILE *f = fopen(filename, "rb");
  if (! f)
    return 1;

  model->vtx = NULL;
  model->indices = NULL;

  if (read_u32_le(f, &model->n_vtx) != 0 || model->n_vtx > INT_MAX)
    goto err;
  
  if (read_u32_le(f, &model->n_tri) != 0 || model->n_tri > INT_MAX)
    goto err;

  size_t vtx_size = 2 * 3 * 4 * model->n_vtx;
  model->vtx = malloc(vtx_size);
  if (! model->vtx)
    goto err;
  if (fread(model->vtx, 1, vtx_size, f) != vtx_size)
    goto err;

  size_t indices_size = 3 * 4 * model->n_tri;
  model->indices = malloc(indices_size);
  if (! model->indices)
    goto err;
  if (fread(model->indices, 1, indices_size, f) != indices_size)
    goto err;
  
  debug("  - %u verts, %u triangles\n", model->n_vtx, model->n_tri);
  
  fclose(f);
  return 0;

 err:
  fclose(f);
  free(model->vtx);
  free(model->indices);
  return 1;
}

int load_model_colors(const char *filename, void (*set_color)(int num, float *color), int max_colors)
{
  FILE *f = fopen(filename, "r");
  if (! f)
    return 0;
  char line[256];
  int num = 0;
  while (num < max_colors && fgets(line, sizeof(line), f) != NULL) {
    float color[3];
    if (sscanf(line, "%f , %f , %f", &color[0], &color[1], &color[2]) == 3) {
      set_color(num, color);
      num++;
    }
  }
  fclose(f);
  return num;
}
