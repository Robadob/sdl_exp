#version 430
//http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/#solution-2--the-3d-way
uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projectionMat;
uniform vec3 _up;
uniform vec3 _right;
uniform vec3 loc;

in vec3 _vertex;
in vec2 _texCoords;

uniform samplerBuffer _texBufX;
uniform samplerBuffer _texBufY;
uniform samplerBuffer _texBufZ;
uniform samplerBuffer _texBufP53;

out vec2 texCoords;

void main () {
  //Correctly apply model matrix to vertex
  vec4 modelVert = _modelMat * vec4(_vertex, 1.0f);
  //modelVert.xyz = modelVert.xyz + loc;
  //Do billboard stuff
  modelVert.xyz = loc + _right*modelVert.x+_up*modelVert.y;
  gl_Position =  _projectionMat * _viewMat * modelVert;//vec4(vertex, 1.0);
  texCoords = _texCoords;
}