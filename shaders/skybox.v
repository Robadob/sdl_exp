#version 430

layout(location = 0) in vec3 _vertex;
//layout(location = 1) in vec3 in_normal;
layout(location = 2) out vec3 tex_coords;

layout(location = 0) uniform mat4 _modelViewMat;
layout(location = 1) uniform mat4 _projectionMat;


void main () {
  tex_coords = _vertex;
  gl_Position = _projectionMat * _modelViewMat * vec4(_vertex, 1.0);
}