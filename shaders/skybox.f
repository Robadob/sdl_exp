#version 430

layout(location = 2) in vec3 tex_coords;

layout(location = 3) uniform samplerCube cube_texture;

out vec4 frag_colour;

void main () {
  frag_colour = texture(cube_texture, tex_coords);
}