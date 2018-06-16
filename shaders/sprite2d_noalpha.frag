#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;

void main()
{
  fragColor = vec4(texture(_texture, vec2(texCoords.x,texCoords.y)).rgb,1.0f);
}