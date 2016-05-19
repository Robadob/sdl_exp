#version 430

in vec3 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;

void main()
{
  //Grab outColor from a uniform eventually
  vec3 outColor = vec3(1.0f,0.0f,0.0f);
  fragColor = vec4(outColor, texture(_texture, vec2(texCoords.x, -texCoords.y)).r);
}