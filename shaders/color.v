#version 430


uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;
in vec3 _vertex;
in vec3 _color;
 

out vec3 colorV;
 
void main() 
{
    colorV = _color;
    gl_Position = _projectionMat * _modelViewMat * vec4(_vertex, 1.0f);
}