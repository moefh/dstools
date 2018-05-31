/* key_camera.h */

#ifndef KEY_CAMERA_H_FILE
#define KEY_CAMERA_H_FILE

struct key_cam {
  float matrix[16];

  float pos[3];
  float theta;
  float phi;
};

void key_cam_calc_matrix(struct key_cam *cam);

void key_cam_move(struct key_cam *cam, float x, float y, float z);
void key_cam_rotate(struct key_cam *cam, float x, float y);

#endif /* KEY_CAMERA_H_FILE */
