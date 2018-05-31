/* mouse_camera.c */

#include <math.h>

#include "mouse_camera.h"
#include "matrix.h"

void mouse_cam_calc_matrix(struct mouse_cam *cam)
{
  float pos[3];
  vec3_load_spherical(pos, cam->radius, cam->theta, cam->phi);
  pos[0] += cam->center[0];
  pos[1] += cam->center[1];
  pos[2] += cam->center[2];

  mat4_look_at(cam->matrix,
               pos[0], pos[1], pos[2],
               cam->center[0], cam->center[1], cam->center[2],
               0.0, 1.0, 0.0);
}

void mouse_cam_move(struct mouse_cam *cam, float x, float y)
{
  float c = cos(cam->theta-M_PI/2);
  float s = sin(cam->theta-M_PI/2);
  float dx = -x;
  float dy = y;
  cam->center[0] += c*dx - s*dy;
  cam->center[2] += s*dx + c*dy;
  mouse_cam_calc_matrix(cam);
}

void mouse_cam_zoom(struct mouse_cam *cam, float x, float y)
{
  cam->radius *= 1.0+y/100.0;
  mouse_cam_calc_matrix(cam);
}

void mouse_cam_rotate(struct mouse_cam *cam, float x, float y)
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
    mouse_cam_calc_matrix(cam);
}
