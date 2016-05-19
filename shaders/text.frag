#version 430

in vec3 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;

void main()
{
  //Grab outColor from a uniform eventually
  vec4 outColor = vec4(0.0f,0.5f,0.0f,1.0f);
  fragColor = outColor*texture(_texture, texCoords.xy).r;
}