/* hkx.h */

#ifndef HKX_H_FILE
#define HKX_H_FILE

#include <stdint.h>
#include <stddef.h>

struct HKX_TYPE;
struct HKX_MEMBER;
struct HKX_INTERFACE;

struct HKX_MEMBER {
  size_t name;
  size_t type;
  uint32_t flags;
  uint32_t byte_offset;
  uint32_t tag;
};

struct HKX_INTERFACE {
  uint32_t type;
  uint32_t flags;
};

struct HKX_TYPE {
  size_t name;
  size_t parent;
  
  uint32_t flags;
  uint32_t subtype_flags;
  uint32_t pointer;
  uint32_t version;
  uint32_t byte_size;
  uint32_t alignment;
  uint32_t abstract_value;

  size_t n_members;
  size_t first_member;

  size_t n_interfaces;
  size_t first_interface;
};

struct HKX_OBJECT {
  size_t type;
  void *value;
  uint32_t count;
};

struct HKX_FILE {
  size_t alloc_type_names;
  size_t n_type_names;
  char **type_names;

  size_t alloc_field_names;
  size_t n_field_names;
  char **field_names;
  
  size_t alloc_types;
  size_t n_types;
  struct HKX_TYPE *types;

  size_t alloc_members;
  size_t n_members;
  struct HKX_MEMBER *members;

  size_t alloc_interfaces;
  size_t n_interfaces;
  struct HKX_INTERFACE *interfaces;

  size_t alloc_objects;
  size_t n_objects;
  struct HKX_OBJECT *objects;
};

int hkx_read(struct HKX_FILE *hkx, void *data, size_t size);
void hkx_free(struct HKX_FILE *hkx);

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
void hkx_dump(void *data, size_t size);

#endif /* HKX_H_FILE */
