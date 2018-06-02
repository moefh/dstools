/* gennormals.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "model.h"

#define MAX_TRI_PER_VERTEX 128

struct vtx_info {
  int n_tri;
  unsigned int tri[MAX_TRI_PER_VERTEX];
};

static int add_vtx_tri(struct vtx_info *vi, unsigned int vtx, unsigned int tri)
{
  if (vi->n_tri >= MAX_TRI_PER_VERTEX) {
    printf("* ERROR: vertex %u is in too many triangles!\n", vtx);
    return 1;
  }
  vi->tri[vi->n_tri++] = tri;
  return 0;
}

static int calc_vtx_info(struct vtx_info *vtx_info, struct model *model)
{
  for (unsigned int vtx = 0; vtx < model->n_vtx; vtx++)
    vtx_info[vtx].n_tri = 0;
    
  for (unsigned int tri = 0; tri < model->n_tri; tri++) {
    for (int i = 0; i < 3; i++) {
      unsigned int vtx = model->indices[3*tri + i];
      if (add_vtx_tri(&vtx_info[vtx], vtx, tri) != 0)
        return 1;
    }
  }
  return 0;
}

static void vec3_cross(float *restrict ret, const float *restrict a, const float *restrict b)
{
  ret[0] = a[1]*b[2] - a[2]*b[1];
  ret[1] = a[2]*b[0] - a[0]*b[2];
  ret[2] = a[0]*b[1] - a[1]*b[0];
}

static float vec3_dot(const float *a, const float *b)
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static void vec3_normalize(float *v)
{
  float s = 1.0/sqrt(vec3_dot(v, v));
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;
}

static void calc_tri_normal(float *normal, unsigned int tri, struct model *model)
{
  unsigned int vi0 = model->indices[3*tri + 0];
  unsigned int vi1 = model->indices[3*tri + 1];
  unsigned int vi2 = model->indices[3*tri + 2];
  
  float *v0 = &model->vtx[3*vi0];
  float *v1 = &model->vtx[3*vi1];
  float *v2 = &model->vtx[3*vi2];

  float a[3] = { v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2] };
  float b[3] = { v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2] };
  vec3_cross(normal, a, b);
  vec3_normalize(normal);
}

int gen_normals(struct model *model)
{
  struct vtx_info *vtx_info = malloc(model->n_vtx * sizeof *vtx_info);
  if (! vtx_info) {
    printf("* ERROR: out of memory for normal calculation\n");
    return 1;
  }

  model->normals = malloc(model->n_vtx * 3 * sizeof(float));
  if (! model->normals)
    goto err;

  if (calc_vtx_info(vtx_info, model) != 0)
    goto err;

  for (unsigned int vtx = 0; vtx < model->n_vtx; vtx++) {
    float *normal = &model->normals[3*vtx];
    if (vtx_info[vtx].n_tri == 0) {
      printf("* WARNING: vertex %u has no triangles!\n", vtx);
      normal[0] = 0.0;
      normal[1] = 1.0;
      normal[2] = 0.0;
    } else {
      // calculate normals for all triangles of vertex
      float normals[MAX_TRI_PER_VERTEX][3];
      for (int i = 0; i < vtx_info[vtx].n_tri; i++)
        calc_tri_normal(normals[i], vtx_info[vtx].tri[i], model);

      // average into final normal
      normal[0] = 0.0;
      normal[1] = 0.0;
      normal[2] = 0.0;
      for (int i = 0; i < vtx_info[vtx].n_tri; i++) {
        normal[0] += normals[i][0];
        normal[1] += normals[i][1];
        normal[2] += normals[i][2];
      }
      vec3_normalize(normal);
    }
  }
  free(vtx_info);

  return 0;
  
 err:
  free(vtx_info);
  return 1;
}
