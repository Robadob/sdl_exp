 #version 430
 
uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;

layout(location = 0) in vec3 _vertex;
layout(location = 1) in vec3 _texcoords;

out vec2 texcoordsV;

void main(){

  texcoordsV = _texcoords.xy;
  gl_Position =  _projectionMat * _modelViewMat * vec4(_vertex, 1.0f);

}