#version 430

in vec3 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;

void main()
{
  fragColor = texture(_texture, vec2(texCoords.x,-texCoords.y));
}