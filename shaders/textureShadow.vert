#version 430

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec3 eyeLightSource;
out vec4 shadowCoord;
out vec2 texCoords;

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projectionMat;
uniform mat3 _normalMat;
uniform mat4 spotlightViewMat;
uniform mat4 spotlightProjectionMat;

uniform vec3 _lightSource;

void main()
{
	//Lighting attribs
	eyeLightSource = vec3(_viewMat*vec4(_lightSource,1));  
	vec4 tVertex = _viewMat * _modelMat * vec4(_vertex,1);   
    eyeVertex = vec3(tVertex);	
    eyeNormal = normalize(_normalMat * _normal);
	shadowCoord = spotlightProjectionMat * spotlightViewMat * _modelMat * vec4(_vertex,1.0f);
	//Convert screen coords to normalised/tex coords (-1 - 1 to 0 - 1)
	shadowCoord /= 2.0f;
	shadowCoord += vec4(0.5);
  texCoords = _texCoords;
	gl_Position = _projectionMat * tVertex;
}