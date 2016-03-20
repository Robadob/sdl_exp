#version 430

uniform mat4 _modelViewProjectionMat;

in vec3 _vertex;
in vec3 _texCoords;

out vec3 texCoords;

void main()
{
  texCoords = _texCoords;
  gl_Position =  _modelViewProjectionMat * vec4(_vertex,1.0f);
}