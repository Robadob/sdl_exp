#version 430

in vec3 u_normal;

uniform vec3 _color;

out vec4 fragColor;
void main()
{
  //Calculate face normal
  vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));//Face Normal
  //This sets the Light source to be the camera
  vec3 L = normalize(vec3(0,0,0)-u_normal);
  vec3 diffuse = _color * max(dot(L, N), 0.0);
  fragColor = vec4(diffuse.xyz,1);
}