/* model.h */

#ifndef MODEL_H_FILE
#define MODEL_H_FILE

struct model {
  unsigned int n_vtx;
  unsigned int alloc_vtx;
  float *vtx;
  float *normals;
  
  unsigned int n_tri;
  unsigned int alloc_tri;
  unsigned int *indices;
};

#endif /* MODEL_H_FILE */
