#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

in uvec4 _boneIDs;
in vec4 _boneWeights;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec2 texCoords;

const int MAX_BONES = 100;
uniform _bones
{
  mat4 transform[MAX_BONES];
} bones;

void main()
{
  mat4  boneTransform =  bones.transform[_boneIDs[0]] * _boneWeights[0];
        boneTransform += bones.transform[_boneIDs[1]] * _boneWeights[1];
        boneTransform += bones.transform[_boneIDs[2]] * _boneWeights[2];
        boneTransform += bones.transform[_boneIDs[3]] * _boneWeights[3];
  gl_Position = _modelViewProjectionMat * boneTransform * vec4(_vertex,1.0f);

  eyeNormal = normalize(_normalMat * (boneTransform * vec4(_normal,0.0f)).rgb) ;
  eyeVertex = (_modelViewMat * vec4(_vertex, 1.0f)).rgb;
  texCoords = _texCoords;
}