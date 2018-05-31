/* main.c */

#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "debug.h"
#include "gl_error.h"
#include "matrix.h"
#include "shader.h"
#include "model.h"
#include "mouse_camera.h"
#include "key_camera.h"

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT  800

#define MAX_SPEED_NORMAL 0.1f
#define MAX_SPEED_TURBO  3.0f

static GLFWwindow *window;

static int fullscreen_mode;
static float mat_projection[16];
static int use_key_cam = 1;
static struct mouse_cam mouse_cam;
static struct key_cam key_cam;
static float vel_front;
static float vel_side;

struct shader_program {
  GLuint prog_id;

  GLint attr_vtx_pos;
  GLint attr_vtx_normal;

  GLint uni_mat_model_view_projection;
  GLint uni_mat_model_view;
  GLint uni_mat_normal;
  GLint uni_color;
  GLint uni_light_pos;

  float mat_model_view_projection[16];
  float mat_model_view[16];
  float mat_normal[9];
  float color[3];
  float light_pos[3];
};
static struct shader_program prog;

struct model_def {
  float color[3];
  const char *filename;
  struct model model;
  GLuint vtx_array_obj;
  GLuint vtx_buf_obj;
  GLuint index_buf_obj;
  int disable_draw;
};
static struct model_def models[] = {
  { {0.0, 0.3, 0.6}, "maps/10-0 Depths.objc" },
  { {0.0, 0.5, 0.4}, "maps/10-1 Undead Burg.objc" },
  { {0.8, 0.7, 0.0}, "maps/10-2 Firelink Shrine.objc" },
  { {1.0, 1.0, 1.0}, "maps/11-0 Painted World of Ariamis.objc" },
  { {0.0, 0.8, 0.0}, "maps/12-0 Darkroot Garden+Basin.objc" },
  { {1.0, 0.0, 1.0}, "maps/12-1 Oolacile.objc" },
  { {0.3, 0.5, 0.0}, "maps/13-0 Catacombs.objc" },
  { {0.3, 0.3, 0.0}, "maps/13-1 Tomb of the Giants.objc" },
  { {0.0, 0.3, 0.4}, "maps/13-2 Ash Lake.objc" },
  { {0.6, 0.4, 0.0}, "maps/14-0 Blighttown+Quelaags Domain.objc" },
  { {0.5, 0.1, 0.1}, "maps/14-1 Demon Ruins+Lost Izalith.objc" },
  { {0.4, 0.2, 0.0}, "maps/15-0 Sens Fortress.objc" },
  { {0.9, 0.9, 0.0}, "maps/15-1 Anor Londo.objc" },
  { {0.3, 0.3, 0.3}, "maps/16-0 New Londo Ruins+Valley of Drakes.objc" },
  { {0.5, 0.6, 0.5}, "maps/17-0 Duke's Archive+Crystal Caves.objc" },
  { {1.0, 1.0, 0.8}, "maps/18-0 Kiln of the first Flame.objc" },
  { {0.6, 0.6, 0.5}, "maps/18-1 Undead Asylum.objc" },
  { {}, NULL }
};

static int get_shader_attr_id(GLint *id, const char *name)
{
  GLint attr_id = glGetAttribLocation(prog.prog_id, name);
  GL_CHECK_ERRORS();
  if (attr_id < 0)
    debug("* WARNING: can't read attribute '%s'\n", name);
  *id = attr_id;
  return 0;
}

static int get_shader_uniform_id(GLint *id, const char *name)
{
  GLint attr_id = glGetUniformLocation(prog.prog_id, name);
  GL_CHECK_ERRORS();
  if (attr_id < 0)
    debug("* WARNING: can't read uniform '%s'\n", name);
  *id = attr_id;
  return 0;
}

static int init_shaders(void)
{
  prog.prog_id = load_program_shader("vert.shader", "frag.shader");
  if (prog.prog_id == 0)
    return 1;

  // load attribute locations
  if (get_shader_attr_id(&prog.attr_vtx_pos, "vtx_pos") != 0)
    return 1;
  if (get_shader_attr_id(&prog.attr_vtx_normal, "vtx_normal") != 0)
    return 1;

  // load uniform locations
  if (get_shader_uniform_id(&prog.uni_mat_model_view_projection, "mat_model_view_projection") != 0)
    return 1;
  if (get_shader_uniform_id(&prog.uni_mat_model_view, "mat_model_view") != 0)
    return 1;
  if (get_shader_uniform_id(&prog.uni_mat_normal, "mat_normal") != 0)
    return 1;
  if (get_shader_uniform_id(&prog.uni_color, "color") != 0)
    return 1;
  if (get_shader_uniform_id(&prog.uni_light_pos, "light_pos") != 0)
    return 1;

  vec3_load(prog.light_pos, 0.0, 20.0, 10.0);
  
  return 0;
}

static int load_models(void)
{
  for (int i = 0; models[i].filename != NULL; i++) {
    struct model_def *def = &models[i];
    if (load_model(&def->model, def->filename) != 0) {
      debug("* ERROR loading model '%s'\n", def->filename);
      return 1;
    }

    GL_CHECK(glGenVertexArrays(1, &def->vtx_array_obj));
    GL_CHECK(glBindVertexArray(def->vtx_array_obj));
    
    GL_CHECK(glGenBuffers(1, &def->vtx_buf_obj));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, def->vtx_buf_obj));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(GLfloat) * def->model.n_vtx, def->model.vtx, GL_STATIC_DRAW));

    GL_CHECK(glVertexAttribPointer(prog.attr_vtx_pos,    3, GL_FLOAT, GL_FALSE, 2*3*sizeof(GLfloat), NULL));
    GL_CHECK(glVertexAttribPointer(prog.attr_vtx_normal, 3, GL_FLOAT, GL_FALSE, 2*3*sizeof(GLfloat), (void *) (3*sizeof(GLfloat))));
    
    GL_CHECK(glGenBuffers(1, &def->index_buf_obj));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, def->index_buf_obj));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint) * def->model.n_tri, def->model.indices, GL_STATIC_DRAW));

    GL_CHECK(glBindVertexArray(0));
  }

  return 0;
}

static void reset_viewport(GLFWwindow *window, int width, int height)
{
  float aspect = (float) width / height;

  glViewport(0, 0, width, height);
  mat4_frustum(mat_projection, -aspect, aspect, -1.0, 1.0, 1.0, 1200.0);
  //float f = 1.3;
  //mat4_frustum(mat_projection, -f, f, -f/aspect, f/aspect, 1.0, 1200.0);
}

static void reset_view(void)
{
  // mouse camera
  vec3_load(mouse_cam.center, 0.0, 0.0, 0.0);
  mouse_cam.radius = 500.0;
  mouse_cam.theta = 4.0*M_PI/3.0;
  mouse_cam.phi = M_PI/5.0;
  mouse_cam_calc_matrix(&mouse_cam);

  // key camera
  key_cam.theta = 5.970390;
  key_cam.phi = -0.048319;
  vec3_load(key_cam.pos, 51.951118,-58.013084,-49.131378);
  key_cam_calc_matrix(&key_cam);
}

static void reset_mouse_pointer(void)
{
  glfwSetInputMode(window, GLFW_CURSOR, use_key_cam ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static void toggle_fullscreen(void)
{
  fullscreen_mode ^= 1;
  //SDL_SetWindowFullscreen(window, (fullscreen_mode) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

static void dump_cameras(void)
{
  console("\n");
  console("- key camera:\n");
  console("pos: %f,%f,%f\n", key_cam.pos[0], key_cam.pos[1], key_cam.pos[2]);
  console("theta: %f\n", key_cam.theta);
  console("phi: %f\n", key_cam.phi);
  console("\n");
  console("- mouse camera:\n");
  console("center: %f,%f,%f\n", mouse_cam.center[0], mouse_cam.center[1], mouse_cam.center[2]);
  console("radius: %f\n", mouse_cam.radius);
  console("theta: %f\n", mouse_cam.theta);
  console("phi: %f\n", mouse_cam.phi);
}

static void toggle_models(int key, int mods)
{
  int n_model;
  switch (key) {
  case GLFW_KEY_1: n_model = 0; break;
  case GLFW_KEY_2: n_model = 1; break;
  case GLFW_KEY_3: n_model = 2; break;
  case GLFW_KEY_4: n_model = 3; break;
  case GLFW_KEY_5: n_model = 4; break;
  case GLFW_KEY_6: n_model = 5; break;
  case GLFW_KEY_7: n_model = 6; break;
  case GLFW_KEY_8: n_model = 7; break;
  case GLFW_KEY_9: n_model = 8; break;
  case GLFW_KEY_0: n_model = 9; break;
  default:
    n_model = 0;
  }

  if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
      || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    n_model += 10;
  
  if (n_model >= 0 && n_model < (int)(sizeof(models)/sizeof(models[0])))
    models[n_model].disable_draw ^= 1;
}

static void draw_model(struct model_def *def)
{
  if (def->disable_draw)
    return;
  
  //console("- drawing model: %d triangles, gl buffer ids (%u, %u) \n", def->model.n_tri, def->vtx_buf_obj, def->index_buf_obj);

  vec3_copy(prog.color, def->color);
  GL_CHECK(glUniform3fv(prog.uni_color, 1, prog.color));

  GL_CHECK(glBindVertexArray(def->vtx_array_obj));
  GL_CHECK(glEnableVertexAttribArray(prog.attr_vtx_pos));
  GL_CHECK(glEnableVertexAttribArray(prog.attr_vtx_normal));
  
  GL_CHECK(glDrawElements(GL_TRIANGLES, 3*def->model.n_tri, GL_UNSIGNED_INT, NULL));
  //GL_CHECK(glDrawElements(GL_TRIANGLES, 3*def->model.n_tri, GL_UNSIGNED_INT, def->model.indices));

  GL_CHECK(glDisableVertexAttribArray(prog.attr_vtx_normal));
  GL_CHECK(glDisableVertexAttribArray(prog.attr_vtx_pos));
}

static void draw_screen(void)
{
  //console("==========================\n");
  //console("PROJECTION:\n"); mat4_dump(mat_projection);
  //console("MOUSE MATRIX:\n"); mat4_dump(mouse_cam.model_view);
  //console("KEYBOARD MATRIX:\n"); mat4_dump(key_cam.model_view);

  // update local uniform values
  mat4_copy(prog.mat_model_view, (use_key_cam) ? key_cam.matrix : mouse_cam.matrix);
  mat3_from_mat4(prog.mat_normal, prog.mat_model_view);  // normal = inverse(transpose(model_view))
  mat4_mul(prog.mat_model_view_projection, mat_projection, prog.mat_model_view);

  // render
  glClearColor(0.0, 0.0, 0.4, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  GL_CHECK(glUseProgram(prog.prog_id));
  GL_CHECK(glUniformMatrix4fv(prog.uni_mat_model_view_projection, 1, GL_TRUE, prog.mat_model_view_projection));
  GL_CHECK(glUniformMatrix4fv(prog.uni_mat_model_view, 1, GL_TRUE, prog.mat_model_view));
  GL_CHECK(glUniformMatrix3fv(prog.uni_mat_normal, 1, GL_TRUE, prog.mat_normal));
  GL_CHECK(glUniform3fv(prog.uni_light_pos, 1, prog.light_pos));

  for (int i = 0; models[i].filename != NULL; i++)
    draw_model(&models[i]);

  GL_CHECK(glUseProgram(0));
  
  glfwSwapBuffers(window);
}

static void mouse_pos_callback(GLFWwindow *window, double x, double y)
{
  static double prev_x, prev_y;
  static int init;

  if (! init) {
    init = 1;
    prev_x = x;
    prev_y = y;
  }
  
  key_cam_rotate(&key_cam, x - prev_x, y - prev_y);
  prev_x = x;
  prev_y = y;
}

static void mouse_scroll_callback(GLFWwindow *window, double dx, double dy)
{
  key_cam_move(&key_cam, dx, 0, dy);
}
  
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;
  
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, 1);
    break;
    
  case GLFW_KEY_1: case GLFW_KEY_2:
  case GLFW_KEY_3: case GLFW_KEY_4:
  case GLFW_KEY_5: case GLFW_KEY_6:
  case GLFW_KEY_7: case GLFW_KEY_8:
  case GLFW_KEY_9: case GLFW_KEY_0:
    toggle_models(key, mods);
    break;
    
  case GLFW_KEY_F11:
    toggle_fullscreen();
    break;
    
  case GLFW_KEY_R:
    reset_view();
    break;
    
  case GLFW_KEY_P:
    dump_cameras();
    break;
    
  case GLFW_KEY_K:
    use_key_cam ^= 1;
    reset_mouse_pointer();
    break;

#if 0
  case GLFW_KEY_W: key_cam_move(&key_cam,  0,  1, 0); break;
  case GLFW_KEY_A: key_cam_move(&key_cam, -1,  0, 0); break;
  case GLFW_KEY_S: key_cam_move(&key_cam,  0, -1, 0); break;
  case GLFW_KEY_D: key_cam_move(&key_cam,  1,  0, 0); break;
#endif
    
  default:
    break;
  }
}

static void process_movement(void)
{
  int turbo = (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
               || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
  float accel = turbo ? 0.02f : 0.01f;
  float max_speed = turbo ? MAX_SPEED_TURBO : MAX_SPEED_NORMAL;
  
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    vel_front += accel;
  else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    vel_front -= accel;
  else
    vel_front *= 0.9;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    vel_side += accel;
  else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    vel_side -= accel;
  else
    vel_side *= 0.9;

  if (vel_front > max_speed)
    vel_front = max_speed;
  else if (vel_front < -max_speed)
    vel_front = -max_speed;
  else if (vel_front > -0.0001 && vel_front < 0.0001)
    vel_front = 0.0;

  if (vel_side > max_speed)
    vel_side = max_speed;
  else if (vel_side < -max_speed)
    vel_side = -max_speed;
  else if (vel_side > -0.0001 && vel_side < 0.0001)
    vel_side = 0.0;

  key_cam_move(&key_cam, vel_side, vel_front, 0); 
}

static int handle_events(void)
{
  glfwPollEvents();
  return glfwWindowShouldClose(window);
}

static int init_gfx(void)
{
  debug("  - Initializing GLFW...\n");
  if (! glfwInit())
    return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "DSView", NULL, NULL);
  if (! window) {
    glfwTerminate();
    debug("* ERROR: can't create window\n");
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, reset_viewport);
  glfwSetCursorPosCallback(window, mouse_pos_callback);
  glfwSetScrollCallback(window, mouse_scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSwapInterval(1);

  debug("  - Initializing OpenGL extensions...\n");
  if (! gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    debug("* ERROR: can't load OpenGL extensions\n");
    return -1;
  }

  return 0;
}

static void cleanup_gfx(void)
{
  glfwTerminate();
}

int main(int argc, char* argv[])
{
  init_debug();

  debug("- Initializing GFX...\n");
  if (init_gfx() != 0)
    return 1;

  int ret = 1;

  debug("- Loading shaders...\n");
  if (init_shaders() != 0)
    goto err;
  
  debug("- Loading models...\n");
  if (load_models() != 0)
    goto err;

  debug("- Setting up view...\n");
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  reset_viewport(window, width, height);
  reset_view();
  reset_mouse_pointer();
  draw_screen();

  debug("- Running main loop...\n");
  while (handle_events() == 0) {
    process_movement();
    draw_screen();
  }
  ret = 0;

 err:
  debug("- Cleaning up GFX...\n");
  cleanup_gfx();

  debug("- Done.\n");
  return ret;
}
