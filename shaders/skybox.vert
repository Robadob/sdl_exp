#version 430

in vec3 _vertex;
out vec3 texCoords;

uniform mat4 _modelViewProjectionMat;


void main () {
  texCoords = _vertex;
  gl_Position = _modelViewProjectionMat * vec4(_vertex, 1.0);
}