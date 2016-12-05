#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;
vec3 hsv2rgb(vec3 hsv)
{
  if(hsv.g==0)//Grey
    return vec3(hsv.b);

  float h = hsv.r/60;
  int i = int(floor(h));
  float f = h-i;
  float p = hsv.b * (1-hsv.g);
  float q = hsv.b * (1-hsv.g * f);
  float t = hsv.b * (1-hsv.g * (1-f));
  switch(i)
  {
    case 0:
      return vec3(hsv.b,t,p);
    case 1:
      return vec3(q,hsv.b,p);
    case 2:
      return vec3(p,hsv.b,p);
    case 3:
      return vec3(p,q,hsv.b);
    case 4:
      return vec3(t,p,hsv.b);
    default: //case 5
      return vec3(hsv.b,p,q);
  }

}
void main()
{
	//Render depth as HSV, giving us better contrast
	float depthColor = texture(_texture, vec2(texCoords.x,texCoords.y)).r;
	fragColor = vec4(hsv2rgb(vec3(depthColor*360,1,1)),1);
}