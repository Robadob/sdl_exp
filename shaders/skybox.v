#version 440

layout(location = 0) in vec3 in_position;
//layout(location = 1) in vec3 in_normal;
layout(location = 2) out vec3 tex_coords;

layout(location = 1) uniform mat4 modelview_matrix;
layout(location = 2) uniform mat4 projection_matrix;


void main () {
  tex_coords = in_position;
  gl_Position = projection_matrix * modelview_matrix * vec4(in_position, 1.0);
}