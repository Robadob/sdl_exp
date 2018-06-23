#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;
uniform mat4 _modelMat;

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

//Shadow attribs
uniform mat4 spotlightViewMat;
uniform mat4 spotlightProjectionMat;

out vec4 shadowCoord;

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

  //Lighting attribs (We can only handle shadows for a couple of lights before we need to do deferred rendering)
  shadowCoord = spotlightProjectionMat * spotlightViewMat * _modelMat * vec4(_vertex,1.0f);
  //Convert screen coords to normalised/tex coords (-1 - 1 to 0 - 1)
  shadowCoord /= 2.0f;
  shadowCoord += vec4(0.5);
}