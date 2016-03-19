#version 130

in vec2 _coords;


out vec3 fragColor;

uniform sampler2D _texture;

void main()
{
  fragColor = texture( _texture, vec2(_coords.x, -_coords.y)).bgr;
}