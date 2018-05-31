#version 150

in vec3 vtx_pos;
in vec3 vtx_normal;

varying vec3 frag_pos;
varying vec3 frag_normal;

uniform mat4 mat_model_view_projection;
uniform mat4 mat_model_view;
uniform mat3 mat_normal;

void main() {
  frag_normal = normalize(mat_normal * vtx_normal);
  frag_pos = vec3(mat_model_view * vec4(vtx_pos, 1.0));
  gl_Position = mat_model_view_projection * vec4(vtx_pos, 1.0);
}
