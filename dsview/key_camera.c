/* key_camera.c */

#include <math.h>

#include "key_camera.h"
#include "matrix.h"

void key_cam_calc_matrix(struct key_cam *cam)
{
  float dir[3];
  vec3_load_spherical(dir, 500.0, cam->theta, cam->phi);

  float center[3] = {
    cam->pos[0] + dir[0],
    cam->pos[1] + dir[1],
    cam->pos[2] + dir[2],
  };
  //center[0] = 0;
  //center[1] = 0;
  //center[2] = 0;
  
  mat4_look_at(cam->matrix,
               cam->pos[0], cam->pos[1], cam->pos[2],
               center[0], center[1], center[2],
               0.0, 1.0, 0.0);
}

void key_cam_move(struct key_cam *cam, float dx, float dy, float dz)
{
  float side[3];
  float dir[3];
  float up[3] = { 0.0, 1.0, 0.0 };
  vec3_load_spherical(dir, 1.0, cam->theta, cam->phi);
  vec3_cross(side, dir, up);
  vec3_normalize(side);
  vec3_cross(up, side, dir);
  vec3_normalize(up);

  cam->pos[0] += dx*side[0] + dy*dir[0] + dz*up[0];
  cam->pos[1] += dx*side[1] + dy*dir[1] + dz*up[1];
  cam->pos[2] += dx*side[2] + dy*dir[2] + dz*up[2];

  key_cam_calc_matrix(cam);
}

void key_cam_rotate(struct key_cam *cam, float x, float y)
{
  int calc = 0;
  
  if (x != 0) {
    cam->theta += x / 200.0;
    calc = 1;
  }

  if (y != 0) {
    cam->phi -= y / 200.0;
    if (cam->phi >= M_PI/2 - 0.05)
      cam->phi = M_PI/2 - 0.05;
    else if (cam->phi <= -M_PI/2 + 0.05)
      cam->phi = -M_PI/2 + 0.05;
    calc = 1;
  }

  if (calc)
    key_cam_calc_matrix(cam);
}
