/* mouse_camera.h */

#ifndef MOUSE_CAMERA_H_FILE
#define MOUSE_CAMERA_H_FILE

struct mouse_cam {
  float matrix[16];

  float center[3];
  float radius; // distance from center
  float theta;  // rotation at ground level (x-z plane)
  float phi;    // elevation from ground
};

void mouse_cam_calc_matrix(struct mouse_cam *cam);

void mouse_cam_move(struct mouse_cam *cam, float x, float y);
void mouse_cam_zoom(struct mouse_cam *cam, float x, float y);
void mouse_cam_rotate(struct mouse_cam *cam, float x, float y);

#endif /* MOUSE_CAMERA_H_FILE */
