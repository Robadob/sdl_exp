#version 430

layout(location = 0) in vec3 _vertex;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec3 _color;
out vec3 o_normal;
out vec3 o_color;

layout(location = 0) uniform mat4 _modelViewMat;
layout(location = 1) uniform mat4 _projectionMat;

void main()
{
  o_normal = (_modelViewMat * vec4(_normal,1.0f)).rgb;
  //apply model view proj
  gl_Position = _projectionMat * vec4(_vertex, 1.0f);
  //Pass gl_Color to frag shader
  o_color = _color;//vec4(_color,1.0);
}