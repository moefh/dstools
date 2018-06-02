/* model.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "model.h"
#include "gennormals.h"
#include "dir.h"

static void free_model(struct model *m)
{
  m->n_vtx = 0;
  m->alloc_vtx = 0;
  m->n_tri = 0;
  m->alloc_tri = 0;
  free(m->vtx);
  free(m->normals);
  free(m->indices);
}

static int add_vertex(struct model *l, float x, float y, float z)
{
  if (l->n_vtx + 1 >= l->alloc_vtx) {
    unsigned int alloc_vtx = (l->alloc_vtx == 0) ? 1024 : 2*l->alloc_vtx;
    float *vtx = realloc(l->vtx, alloc_vtx * 3 * sizeof(float));
    if (! vtx)
      return 1;
    l->alloc_vtx = alloc_vtx;
    l->vtx = vtx;
  }
  l->vtx[3*l->n_vtx + 0] = x;
  l->vtx[3*l->n_vtx + 1] = y;
  l->vtx[3*l->n_vtx + 2] = z;
  l->n_vtx++;
  return 0;
}

static int add_index(struct model *l, unsigned int f1, unsigned int f2, unsigned int f3)
{
  if (l->n_tri + 1 >= l->alloc_tri) {
    unsigned int alloc_tri = (l->alloc_tri == 0) ? 1024 : 2*l->alloc_tri;
    unsigned int *indices = realloc(l->indices, alloc_tri * 3 * sizeof(unsigned int));
    if (! indices)
      return 1;
    l->alloc_tri = alloc_tri;
    l->indices = indices;
  }
  l->indices[3*l->n_tri + 0] = f1;
  l->indices[3*l->n_tri + 1] = f2;
  l->indices[3*l->n_tri + 2] = f3;
  l->n_tri++;
  return 0;
}

static int load_model(struct model *model, const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (! f)
    return 1;

  model->n_vtx = 0;
  model->alloc_vtx = 0;
  model->vtx = NULL;
  model->normals = NULL;
  model->n_tri = 0;
  model->alloc_tri = 0;
  model->indices = NULL;
  
  int line_num = 0;
  while (1) {
    char line[1024];
    if (fgets(line, sizeof(line), f) == NULL)
      break;
    line_num++;

    char *p = line;
    while (isspace(*p))
      p++;
    if (*p == '#' || *p == '\0')
      continue;

    if (*p == 'v') {
      float x, y, z;
      if (sscanf(p, "v %f %f %f", &x, &y, &z) != 3) {
	printf("* ERROR in '%s', line %d: invalid vertex spacification\n", filename, line_num);
	goto err;
      }
      if (add_vertex(model, x, y, z) != 0) {
	printf("* ERROR in '%s', line %d: out of memory\n", filename, line_num);
	goto err;
      }
      continue;
    }

    if (*p == 'f') {
      unsigned int f1, f2, f3;
      if (sscanf(p, "f %u %u %u", &f1, &f2, &f3) != 3) {
	printf("* ERROR in '%s', line %d: invalid face spacification\n", filename, line_num);
	goto err;
      }
      if (f1 == 0 || f2 == 0 || f3 == 0) {
	printf("* ERROR in '%s', line %d: invalid index\n", filename, line_num);
	goto err;
      }
      if (add_index(model, f1-1, f2-1, f3-1) != 0) {
	printf("* ERROR in '%s', line %d: out of memory\n", filename, line_num);
	goto err;
      }
      continue;
    }
    
    printf("* ERROR in '%s', line %d: invalid line\n", filename, line_num);
    goto err;
  }

  if (gen_normals(model) != 0)
    goto err;
  
  fclose(f);
  return 0;

 err:
  fclose(f);
  free(model->vtx);
  free(model->indices);
  return 1;
}

static int write_model(const struct model *model, const char *filename)
{
  FILE *f = fopen(filename, "wb");
  if (! f)
    return 1;

  if (fwrite(&model->n_vtx, 1, 4, f) != 4) {
    fclose(f);
    printf("* ERROR writing number of vertices\n");
    return 1;
  }
  if (fwrite(&model->n_tri, 1, 4, f) != 4) {
    fclose(f);
    printf("* ERROR writing number of triangles\n");
    return 1;
  }

  for (unsigned int i = 0; i < model->n_vtx; i++) {
    if (fwrite(&model->vtx[3*i], 1, 3*4, f) != 3*4) {
      printf("* ERROR writing vertex %d\n", i);
      goto err;
    }
    if (fwrite(&model->normals[3*i], 1, 3*4, f) != 3*4) {
      printf("* ERROR writing normal %d\n", i);
      goto err;
    }
  }

  if (fwrite(model->indices, 1, 3*4*model->n_tri, f) != 3*4*model->n_tri) {
    fclose(f);
    printf("* ERROR writing indices\n");
    return 1;
  }
  fclose(f);
  return 0;

 err:
  fclose(f);
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("USAGE: genmap input_dir output_dir\n");
    return 1;
  }
  const char *input_dir = argv[1];
  const char *output_dir = argv[2];
  
  char **model_files = dir_list_files(input_dir, ".obj");
  if (! model_files) {
    printf("* ERROR listing directory '%s'\n", input_dir);
    return 1;
  }
  for (int i = 0; model_files[i] != NULL; i++) {
    char out_filename[256];
    struct model model;
    
    printf("- processing '%s'...\n", model_files[i]);

    if (load_model(&model, model_files[i]) != 0) {
      printf("* ERROR loading '%s'\n", model_files[i]);
      return 1;
    }

    char *filename = get_path_filename(model_files[i]);
    snprintf(out_filename, sizeof(out_filename), "%s/%sc", output_dir, filename);
    if (write_model(&model, out_filename) != 0) {
      printf("* ERROR writing '%s'\n", out_filename);
      return 1;
    }

    free_model(&model);
  }
  
  return 0;
}
