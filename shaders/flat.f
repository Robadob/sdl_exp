#version 120

varying vec3 u_normal;
void main()
{
  //Calculate face normal
  vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));//Face Normal
  //This sets the Light source to be the camera
  vec3 L = normalize(vec3(0,0,0)-u_normal);
  vec3 diffuse = gl_Color.xyz * max(dot(L, N), 0.0);
  gl_FragColor = vec4(diffuse.xyz,1);
}