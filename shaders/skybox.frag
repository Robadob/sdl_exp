#version 430

in vec3 tex_coords;

uniform samplerCube _texture;

out vec4 frag_colour;

void main () {
  frag_colour = texture(_texture, tex_coords);
}