#version 430

in vec2 texCoords;

out vec3 fragColor;

uniform sampler2D _texture;

void main()
{
  fragColor = texture( _texture, vec2(texCoords.x, texCoords.y)).rgb;
}