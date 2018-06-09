#version 430

uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;

in vec3 _vertex;
in vec2 _texCoords;

out vec3 eyeVertex;
out vec2 texCoords;

void main()
{
  gl_Position = _modelViewProjectionMat * vec4(_vertex,1.0f);

  eyeVertex = (_modelViewMat * vec4(_vertex, 1.0f)).rgb;
  texCoords = _texCoords;
}