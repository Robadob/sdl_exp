#version 430

uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;

in vec3 _vertex;
in vec3 _color;
in vec3 _normal;

out vec4 o_color;
out vec3 o_normal;
out vec3 o_vertex;
 
void main() 
{
    o_normal = _normal;
    o_vertex = _vertex;
    o_color = vec4(_color,1.0f);
    gl_Position = _projectionMat * _modelViewMat * vec4(_vertex, 1.0f);
}