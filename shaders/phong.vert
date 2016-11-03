#version 430
in vec3 _vertex;
in vec3 _normal;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec3 eyeLightSource;

uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;
uniform mat4 _cameraMat;
uniform mat3 _normalMat;

uniform vec3 _lightSource;


void main(void)  
{     
	eyeLightSource = vec3(_cameraMat*vec4(_lightSource,1));  
	vec4 tVertex = _modelViewMat * vec4(_vertex,1);   
    eyeVertex = vec3(tVertex);	
    eyeNormal = normalize(_normalMat * _normal);
	
    gl_Position = _projectionMat * tVertex;
}