/* hkx.h */

#ifndef HKX_H_FILE
#define HKX_H_FILE

struct HKX_GEOMETRY {
  uint32_t alloc_vtx;
  uint32_t alloc_tri;
  uint32_t n_vtx;
  uint32_t n_ind;
  float *vtx;
  uint32_t *ind;
};

void hkx_init_geometry(struct HKX_GEOMETRY *g);
void hkx_free_geometry(struct HKX_GEOMETRY *g);
int hkx_read_geometry(struct HKX_GEOMETRY *restrict g, const void *restrict data, size_t size);
int hkx_write_obj(const char *filename, struct HKX_GEOMETRY *g);

#endif /* HKX_H_FILE */
