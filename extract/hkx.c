/* hkx.c */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hkx.h"
#include "reader.h"

#define HKX_TYPE_MAT 0x1000004b
#define HKX_TYPE_VTX 0x20000016
#define HKX_TYPE_IND 0x2000000d

#define REALLOC_CHUNK_SIZE 16384

void hkx_init_geometry(struct HKX_GEOMETRY *g)
{
  g->alloc_vtx = 0;
  g->alloc_tri = 0;
  g->n_vtx = 0;
  g->n_ind = 0;
  g->vtx = NULL;
  g->ind = NULL;
}

void hkx_free_geometry(struct HKX_GEOMETRY *g)
{
  free(g->vtx);
  free(g->ind);
  g->vtx = NULL;
  g->ind = NULL;
}

static uint32_t grow_alloc_space(uint32_t alloc, uint32_t num)
{
  if (alloc == 0) {
    alloc = num;
  } else {
    if (2 * alloc < alloc + num)
      alloc += num;
    else
      alloc *= 2;
  }

  return (alloc + REALLOC_CHUNK_SIZE - 1) / REALLOC_CHUNK_SIZE * REALLOC_CHUNK_SIZE;
}

static int ensure_vtx_space(struct HKX_GEOMETRY *g, uint32_t num)
{
  if (num > g->alloc_vtx) {
    g->alloc_vtx = grow_alloc_space(g->alloc_vtx, num);
    //printf("growing vtx space to %u\n", (unsigned) g->alloc_vtx);
    void *vtx = realloc(g->vtx, 3 * sizeof(float) * g->alloc_vtx);
    if (! vtx)
      return 1;
    g->vtx = vtx;
  }
  return 0;
}

static int ensure_tri_space(struct HKX_GEOMETRY *g, uint32_t num)
{
  if (num > g->alloc_tri) {
    g->alloc_tri = grow_alloc_space(g->alloc_tri, num);
    //printf("growing tri space to %u\n", (unsigned) g->alloc_tri);
    void *ind = realloc(g->ind, /* 3 * */ sizeof(uint32_t) * g->alloc_tri);
    if (! ind)
      return 1;
    g->ind = ind;
  }
  return 0;
}

int hkx_read_geometry(struct HKX_GEOMETRY *restrict g, const void *restrict data, size_t size)
{
  // get chunk offsets
  uint32_t indx_off = 0;
  uint32_t indx_size = 0;
  uint32_t data_off = 0;
  //uint32_t data_size = 0;
  uint32_t off = 8;
  while (off < size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    if (memcmp(magic, "DATA", 4) == 0) {
      data_off = off + 8;
      //data_size = chunk_size - 8;
      //dump_mem((char *) data + data_off, data_size);
    } else if (memcmp(magic, "INDX", 4) == 0) {
      indx_off = off + 8;
      indx_size = chunk_size - 8;
    }
    off += chunk_size;
  }

  uint32_t n_vtx_start = 0;
  float mat[16] = { 0.0 };

  off = indx_off;
  while (off < indx_off+indx_size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    if (memcmp(magic, "ITEM", 4) == 0) {
      for (uint32_t item = 0; 8 + 12*(item+1) <= chunk_size; item++) {
        uint32_t item_type  = get_u32_le(data, off + 8 + 12*item + 0);
        uint32_t item_off   = get_u32_le(data, off + 8 + 12*item + 4);
        uint32_t item_count = get_u32_le(data, off + 8 + 12*item + 8);
        uint32_t n_ind = 0;
        
        switch (item_type) {
        case HKX_TYPE_MAT:
          memcpy(mat, (char *) data + data_off + item_off + 0x170, 16 * sizeof(float));
          break;
          
        case HKX_TYPE_VTX:
          if (ensure_vtx_space(g, g->n_vtx + item_count) != 0)
            return 1;
          n_vtx_start = g->n_vtx;
          for (uint32_t i = 0; i < item_count; i++) {
            float v[3];
            memcpy(v, (char *) data + data_off + item_off + 4 * sizeof(float) * i, 3 * sizeof(float));
            g->vtx[3 * (g->n_vtx + i) + 0] = v[0]*mat[ 0] + v[1]*mat[ 4] + v[2]*mat[ 8] + mat[12];
            g->vtx[3 * (g->n_vtx + i) + 1] = v[0]*mat[ 1] + v[1]*mat[ 5] + v[2]*mat[ 9] + mat[13];
            g->vtx[3 * (g->n_vtx + i) + 2] = v[0]*mat[ 2] + v[1]*mat[ 6] + v[2]*mat[10] + mat[14];
          }
          g->n_vtx += item_count;
          break;
          
        case HKX_TYPE_IND:
          n_ind = item_count/4*3;
          if (ensure_tri_space(g, g->n_ind + n_ind) != 0)
            return 1;
          for (uint32_t i = 0; 3*i < n_ind; i++) {
            g->ind[g->n_ind + 3 * i + 0] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 0) * sizeof(uint16_t));
            g->ind[g->n_ind + 3 * i + 1] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 1) * sizeof(uint16_t));
            g->ind[g->n_ind + 3 * i + 2] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 2) * sizeof(uint16_t));
          }
          g->n_ind += n_ind;
          break;
        }
      }
    }
    off += chunk_size;
  }

  return 0;
}

int hkx_write_obj(const char *filename, struct HKX_GEOMETRY *g)
{
  FILE *f = fopen(filename, "w");
  if (! f)
    return 1;

  fprintf(f, "# %u vertices\n", g->n_vtx);
  for (uint32_t i = 0; i < g->n_vtx; i++)
    fprintf(f, "v %f %f %f\n", g->vtx[3*i + 0], g->vtx[3*i + 1], g->vtx[3*i + 2]);

  fprintf(f, "\n");

  fprintf(f, "# %u triangles\n", g->n_ind/3);
  for (uint32_t i = 0; 3*i < g->n_ind; i++)
    fprintf(f, "f %u %u %u\n", g->ind[3*i + 0]+1, g->ind[3*i + 1]+1, g->ind[3*i + 2]+1);

  fclose(f);
  return 0;
}

