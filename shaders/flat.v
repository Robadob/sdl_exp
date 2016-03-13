#version 120

varying vec3 u_normal;
void main()
{
  //pass gl_Vertex to frag shader to calculate face normal
  u_normal = (gl_ModelViewMatrix * gl_Vertex).rgb;
  //apply model view proj
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  //Pass gl_Color to frag shader
  gl_FrontColor = gl_Color;
}