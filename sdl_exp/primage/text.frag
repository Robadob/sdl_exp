#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;
uniform vec4 _col;
uniform vec4 _backCol;
void main()
{
  //TextureLod(0) so even if mipmaps gen, we try to use best quality
  float tex = textureLod(_texture, vec2(-texCoords.y, -texCoords.x),0.0f).r;
  //Grab a solid pixel, ensure no interpolation
  //ivec2 texDim = textureSize(_texture, 0);
  //float tex = texelFetch(_texture, ivec2(texDim.y-int(texCoords.y*texDim.y),texDim.x-int(texCoords.x*texDim.x)),0).r;
  //Interpolate the forecolour according to the tex
  vec4 foreground = _col*tex;
  //Interpolate the background according to inverse tex
  vec4 background = _backCol*(1-tex);
  //Blend the colors according to their own alpha channels
  vec3 blend = (foreground.xyz*foreground.w) + (background.xyz*background.w);
  //Output the blended rgb values with a summed alpha channel
  //fragColor = vec4(textureLod(_texture, vec2(texCoords.x,-texCoords.y),0.0f).r, 0.0f, 0.0f, 1.0f); 
  //fragColor = vec4(blend, (foreground.w+background.w));
  fragColor = vec4(1,0,0,1);
}