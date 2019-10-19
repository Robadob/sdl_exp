#version 430

uniform mat3 _normalMat;
uniform mat4 _projectionMat;
uniform mat4 _viewMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

uniform samplerBuffer _texBufX;
uniform samplerBuffer _texBufY;
uniform samplerBuffer _texBufZ;

out vec3 eyeVertex;
out vec3 eyeUNormal;
out vec2 texCoords;

void main()
{
  //Grab model offset from texture array
  vec3 loc_data;
  loc_data.x = texelFetch(_texBufX, gl_InstanceID).x;
  loc_data.y = texelFetch(_texBufY, gl_InstanceID).x;
  loc_data.z = texelFetch(_texBufZ, gl_InstanceID).x;
  
  vec3 vert = (_modelMat*vec4(_vertex, 1.0f)).xyz;
  gl_Position = _projectionMat * _viewMat * vec4(loc_data+vert,1.0f);

  eyeUNormal = normalize(_normalMat * _normal) ;
  eyeVertex = (_viewMat * vec4(loc_data+_vertex, 1.0f)).rgb;
  texCoords = _texCoords;
}