#version 430

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projectionMat;
uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

in uvec4 _boneIDs;
in vec4 _boneWeights;

out vec3 normal;
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
        //boneTransform = glm::mat4(1);
  //gl_Position = _projectionMat * _viewMat * _modelMat * boneTransform * vec4(_vertex,1.0f);
  gl_Position = _modelViewProjectionMat * boneTransform * vec4(_vertex,1.0f);
        
  normal = normalize(_normalMat * (boneTransform * vec4(normal,0.0f)).rgb) ;
    
  texCoords = _texCoords;
}