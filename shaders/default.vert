#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;
in vec3 _tangent;
in vec3 _bitangent;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec2 texCoords;
out mat3 tbnMat;

void main()
{
  gl_Position = _modelViewProjectionMat * vec4(_vertex,1.0f);

  eyeNormal = normalize(_normalMat * _normal);
  eyeVertex = (_modelViewMat * vec4(_vertex, 1.0f)).rgb;
  texCoords = _texCoords;
  vec3 eyeT = normalize(_normalMat * _tangent);
  vec3 eyeB = normalize(_normalMat * _bitangent);
  tbnMat = mat3(eyeT,eyeB,eyeNormal);
}