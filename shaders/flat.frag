#version 120

varying vec3 u_normal;
varying vec3 o_color;
void main()
{
  //Calculate face normal
  vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));//Face Normal
  //This sets the Light source to be the camera
  vec3 L = normalize(vec3(0,0,0)-u_normal);
  vec3 t_color = (gl_Color.xyz==vec3(0.0f))?o_color:gl_Color.xyz;
  vec3 diffuse = t_color * max(dot(L, N), 0.0);
  gl_FragColor = vec4(diffuse.xyz,1);
}