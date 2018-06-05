/* hkx.c */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "hkx.h"
#include "reader.h"
#include "dump.h"

#define HKX_TYPE_BODY 0x00004b
#define HKX_TYPE_VTX  0x000016
#define HKX_TYPE_IND  0x00000d

#define TBOD_SUBTYPE      0x01
#define TBOD_POINTER      0x02
#define TBOD_VERSION      0x04
#define TBOD_BYTESIZE     0x08
#define TBOD_ABSTRACT_VAL 0x10
#define TBOD_MEMBERS      0x20
#define TBOD_INTERFACES   0x40
#define TBOD_UNKNOWN      0x80

static const struct {
  const char *name;
  uint8_t val;
} tbod_flag_names[] = {
#define ADD_FLAG(n)  { #n, TBOD_##n }
  ADD_FLAG(SUBTYPE),
  ADD_FLAG(POINTER),
  ADD_FLAG(VERSION),
  ADD_FLAG(BYTESIZE),
  ADD_FLAG(ABSTRACT_VAL),
  ADD_FLAG(MEMBERS),
  ADD_FLAG(INTERFACES),
  ADD_FLAG(UNKNOWN),
#undef ADD_FLAG
};

#define REALLOC_CHUNK_SIZE 16384
#define REMOVE_INVALID_TRIANGLES 1

/* =======================================================================
 * GEOMETRY
 * =======================================================================
 */

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

  uint32_t n_vtx_start = UINT32_MAX;
  float mat[16] = { 0.0 };

  off = indx_off;
  while (off < indx_off+indx_size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    if (memcmp(magic, "ITEM", 4) == 0) {
      for (uint32_t item = 0; 8 + 12*(item+1) <= chunk_size; item++) {
        uint32_t item_type  = get_u32_le(data, off + 8 + 12*item + 0) & 0x00ffffff;
        uint32_t item_off   = get_u32_le(data, off + 8 + 12*item + 4);
        uint32_t item_count = get_u32_le(data, off + 8 + 12*item + 8);
        uint32_t n_ind = 0;
        
        switch (item_type) {
        case HKX_TYPE_BODY:
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
          if (n_vtx_start == UINT32_MAX) {
            //printf("* ERROR: indices without vertices\n");
            continue;
          }
          n_ind = item_count/4*3;
          if (ensure_tri_space(g, g->n_ind + n_ind) != 0)
            return 1;
          for (uint32_t i = 0; 3*i < n_ind; i++) {
#if REMOVE_INVALID_TRIANGLES
            uint32_t v0 = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 0) * sizeof(uint16_t));
            uint32_t v1 = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 1) * sizeof(uint16_t));
            uint32_t v2 = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 2) * sizeof(uint16_t));
            if (v0 < g->n_vtx && v1 < g->n_vtx && v2 < g->n_vtx) {
              g->ind[g->n_ind + 3 * i + 0] = v0;
              g->ind[g->n_ind + 3 * i + 1] = v1;
              g->ind[g->n_ind + 3 * i + 2] = v2;
            }
#else
            g->ind[g->n_ind + 3 * i + 0] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 0) * sizeof(uint16_t));
            g->ind[g->n_ind + 3 * i + 1] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 1) * sizeof(uint16_t));
            g->ind[g->n_ind + 3 * i + 2] = n_vtx_start + get_u16_le(data, data_off + item_off + (4 * i + 2) * sizeof(uint16_t));
#endif
          }
          g->n_ind += n_ind;
          n_vtx_start = UINT32_MAX;
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
    fprintf(f, "v %f %f %f\n", g->vtx[3*i + 0], g->vtx[3*i + 1], -g->vtx[3*i + 2]);

  fprintf(f, "\n");

  fprintf(f, "# %u triangles\n", g->n_ind/3);
  for (uint32_t i = 0; 3*i < g->n_ind; i++)
    fprintf(f, "f %u %u %u\n", g->ind[3*i + 0]+1, g->ind[3*i + 1]+1, g->ind[3*i + 2]+1);

  fclose(f);
  return 0;
}

/* =======================================================================
 * DUMP
 * =======================================================================
 */

static void dump_tbod_flags(uint32_t flags)
{
  int cont = 0;
  for (size_t i = 0; i < sizeof(tbod_flag_names)/sizeof(tbod_flag_names[0]); i++) {
    if (flags & tbod_flag_names[i].val) {
      flags &= ~tbod_flag_names[i].val;
      if (cont)
        printf("|");
      else
        cont = 1;
      printf("%s", tbod_flag_names[i].name);
    }
  }
  if (flags) {
    if (cont)
      printf("|");
    printf("%x", flags);
  }
}

static void dump_index(void *data, size_t size)
{
  printf("\nItems in index:\n");
  uint32_t off = 0;
  while (off < size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    void *content = (char *) data + off + 8;
    uint32_t content_size = chunk_size - 8;
    if (memcmp(magic, "ITEM", 4) == 0) {
      printf("type     offset   count\n");
      for (uint32_t item = 0; 12*(item+1) <= content_size; item++) {
        uint32_t item_type  = get_u32_le(content, 12*item + 0) & 0x00ffffff;
        uint32_t item_off   = get_u32_le(content, 12*item + 4);
        uint32_t item_count = get_u32_le(content, 12*item + 8);
        printf("[%06x] %08x %08x\n", item_type, item_off, item_count);
      }
    }
    off += chunk_size;
  }
}

static void dump_type(void *data, size_t size)
{
  char *type_names[256] = { NULL };
  char *field_names[512] = { NULL };
  
  uint32_t off = 0;
  while (off < size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    void *content = (char *) data + off + 8;
    uint32_t content_size = chunk_size - 8;
    if (memcmp(magic, "TSTR", 4) == 0) {
      printf("\nTYPE/TSTR:\n");
      uint32_t id = 0;
      char *name = content;
      while (name - (char *) content < content_size) {
        if (name[0] != '\0')
          printf("  %06x %s\n", id, name);
        if (id < sizeof(type_names)/sizeof(type_names[0]))
          type_names[id++] = name;
        name += strlen(name) + 1;
      }
    } else if (memcmp(magic, "FSTR", 4) == 0) {
      printf("\nTYPE/FSTR:\n");
      uint32_t id = 0;
      char *name = content;
      while (name - (char *) content < content_size) {
        if (name[0] != '\0')
          printf("  %06x %s\n", id, name);
        if (id < sizeof(field_names)/sizeof(field_names[0]))
          field_names[id++] = name;
        name += strlen(name) + 1;
      }
    } else if (memcmp(magic, "TNAM", 4) == 0) {
      printf("\nTYPE/TNAM:\n");
      size_t offset = 0;
      uint32_t num_types = get_packed(content, &offset);
      for (uint32_t type_num = 1; type_num < num_types; type_num++) {
        uint32_t type = get_packed(content, &offset);
        uint32_t num_vals = get_packed(content, &offset);
        printf("[%06x] %08x (%s)\n",
               type_num,
               type,
               (type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[type] : "?");
        for (uint32_t val_i = 0; val_i < num_vals; val_i++) {
          uint32_t t_nam = get_packed(content, &offset);
          uint32_t t_val = get_packed(content, &offset);
          printf("    %08x -> %08x\n", t_nam, t_val);
        }
      }
    } else if (memcmp(magic, "TBOD", 4) == 0) {
      printf("\nTYPE/TBOD:\n");
      size_t offset = 0;
      while (offset < content_size) {
        uint32_t type = get_packed(content, &offset);
        if (type == 0)
          continue;
        uint32_t parent_type = get_packed(content, &offset);
        uint32_t flags = get_packed(content, &offset);
        uint32_t subtype_flags = 0;

        printf("%08x (%s)\n"
               "    parent:  %08x (%s)\n",
               type,
               (type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[type] : "?",
               parent_type,
               (parent_type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[parent_type] : "?");
        printf("    flags: %08x (", flags);
        dump_tbod_flags(flags);
        printf(")\n");
        
        if (flags & TBOD_SUBTYPE) {
          subtype_flags = get_packed(content, &offset);
          printf("    subtype: %08x\n", subtype_flags);
        }

        if ((flags & TBOD_POINTER) && (subtype_flags & 0x0f) >= 6) {
          uint32_t ptr_type = get_packed(content, &offset);
          printf("    pointer: %08x (%s)\n",
                 ptr_type,
                 (ptr_type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[ptr_type] : "?");
        } else if (flags & TBOD_POINTER) {
          printf("    pointer with NO TYPE!\n");
        }

        if (flags & TBOD_VERSION) {
          uint32_t version = get_packed(content, &offset);
          printf("    version: %08x\n", version);
        }

        if (flags & TBOD_BYTESIZE) {
          uint32_t byte_size = get_packed(content, &offset);
          uint32_t byte_align = get_packed(content, &offset);
          printf("    byte_size:  %08x\n"
                 "    byte_align: %08x\n",
                 byte_size,
                 byte_align);
        }

        if ((subtype_flags & 0x1e0) && (flags & TBOD_POINTER) == 0) {
          uint32_t unknown = get_packed(content, &offset);
          printf("    UNKNOWN: %08x\n", unknown);
        }

        if (flags & TBOD_ABSTRACT_VAL) {
          uint32_t abstract_val = get_packed(content, &offset);
          printf("    abstract_val: %08x\n", abstract_val);
        }

        if (flags & TBOD_MEMBERS) {
          uint32_t n_members = get_packed(content, &offset);
          printf("    members: (%u)\n", n_members);
          for (uint32_t i = 0; i < n_members; i++) {
            uint32_t memb_name = get_packed(content, &offset);
            uint32_t memb_flags = get_packed(content, &offset);
            uint32_t memb_off = get_packed(content, &offset);
            uint32_t memb_type = get_packed(content, &offset);
            printf("        %08x (%s)\n"
                   "        type:  %08x (%s)\n"
                   "        flags: %08x\n"
                   "        off:   %08x\n",
                   memb_name,
                   (memb_name < sizeof(field_names)/sizeof(field_names[0])) ? field_names[memb_name] : "?",
                   memb_type,
                   (memb_type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[memb_type] : "?",
                   memb_flags,
                   memb_off);
          }
        }

        if (flags & TBOD_INTERFACES) {
          uint32_t n_inter = get_packed(content, &offset);
          printf("    interfaces: (%u)\n", n_inter);
          for (uint32_t i = 0; i < n_inter; i++) {
            uint32_t inter_type = get_packed(content, &offset);
            uint32_t inter_flags = get_packed(content, &offset);
            printf("        %08x (%s)\n"
                   "        flags: %08x\n",
                   inter_type,
                   (inter_type < sizeof(type_names)/sizeof(type_names[0])) ? type_names[inter_type] : "?",
                   inter_flags);
          }
        }
      }
    }
    off += chunk_size;
  }
}

void hkx_dump(void *data, size_t size)
{
  uint32_t off = 8;

  while (off < size) {
    uint32_t chunk_size = get_u32_be(data, off) & 0x00ffffff;
    char *magic = (char *) data + off + 4;
    void *content = (char *) data + off + 8;
    uint32_t content_size = chunk_size - 8;
    printf("\n");
    printf("%08x %.4s len=%u (0x%x)\n", off, magic, content_size, content_size);
    dump_mem(content, content_size, off + 8);

    if (memcmp(magic, "INDX", 4) == 0) {
      dump_index(content, content_size);
    } else if (memcmp(magic, "TYPE", 4) == 0) {
      dump_type(content, content_size);
    }
    
    off += chunk_size;
  }
  if (off != size)
    printf("warning: off != size (0x%08x != 0x%08x)\n", off, (uint32_t) size);
  printf("\n");
}
