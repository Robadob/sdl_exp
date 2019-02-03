#version 430

in vec3 texCoords;

uniform samplerCube t_diffuse;

out vec4 frag_colour;

void main () {
  //Skybox is fullbright
  frag_colour = texture(t_diffuse, texCoords);
  //Skybox always writes to back of depth buffer
  gl_FragDepth = 1.0f;
}