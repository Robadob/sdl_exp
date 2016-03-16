#version 430
in vec3 o_normal;
in vec3 o_color;

void main()
{
  //This sets the Light source to be the camera
  vec3 L = normalize(vec3(0,0,0)-o_normal);
  vec3 diffuse = o_color * max(dot(L, o_normal), 0.0);
  gl_FragColor = vec4(diffuse,1);
}