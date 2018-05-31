#version 150

varying vec3 frag_pos;
varying vec3 frag_normal;

out vec3 frag;
uniform vec3 color;
uniform vec3 light_pos;

void main() {
  vec3 light_dir = normalize(light_pos - frag_pos);
  float diffuse = 0.3 + 0.7*max(dot(frag_normal, light_dir), 0.0);

  vec3 view_dir = normalize(vec3(0.0,0.0,0.0) - frag_pos);
  vec3 reflect_dir = reflect(-light_dir, frag_normal);
  float specular = 0.5 * pow(max(dot(view_dir, reflect_dir), 0.0), 32);

  frag = color * clamp(diffuse + specular, 0.0, 1.0);
}
