#version 430

in vec3 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;
uniform vec4 _col;
uniform vec4 _backCol;

void main()
{
  float tex = texture(_texture, vec2(texCoords.x,-texCoords.y)).r;
  //Interpolate the forecolour according to the tex
  vec4 foreground = _col*tex;
  //Interpolate the background according to inverse tex
  vec4 background = _backCol*(1-tex);
  //Blend the colors according to their own alpha channels
  vec3 blend = (foreground.xyz*foreground.w) + (background.xyz*background.w);
  //Output the blended rgb values with a summed alpha channel
  fragColor = vec4(blend, foreground.w+background.w);
}