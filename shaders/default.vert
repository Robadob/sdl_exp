#version 430

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

out vec3 vertex;
out vec3 normal;
out vec2 texCoords;

uniform mat4 _modelViewProjectionMat;

void main()
{
	vertex = _vertex;
	normal = _normal;
	texCoords = _texCoords;
	gl_Position =  _modelViewProjectionMat * vec4(_vertex,1.0f);
}