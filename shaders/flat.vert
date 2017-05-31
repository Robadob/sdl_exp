#version 430

uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;

in vec3 _vertex;

out vec3 u_normal;

void main()
{
  //pass _vertex to frag shader to calculate face normal
  u_normal = (_modelViewMat * vec4(_vertex,1.0f)).rgb;
  //apply model view proj
  gl_Position = _projectionMat * _modelViewMat * vec4(_vertex,1.0f);
}