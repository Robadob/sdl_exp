#version 430

uniform mat4 _modelViewProjectionMat;
in vec3 _vertex;
in vec4 _color;
in vec3 _normal;

out vec4 o_color;
out vec3 o_normal;
out vec3 o_vertex;
 
void main() 
{
    o_normal = _normal;
    o_vertex = _vertex;
    o_color = _color;//if GL_BLEND disabled, will act as RGB anyway
    gl_Position = _modelViewProjectionMat * vec4(_vertex, 1.0f);
}