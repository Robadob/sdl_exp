#version 430

in vec3 texCoords;

in float distOrigin;
in flat int renderOrder;
out vec4 fragColor;
//HSV interpolation is more attractive than rgb interpolation
//http://www.cs.rit.edu/~ncs/color/t_convert.html
//hsv(0-360,0-1,0-1)
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
float lerp(float a, float b, float w)
{
  return float((1.0f-w)*a+w*b);
}
void main()
{ 
  //tex cords run from 0 to +1
  //Normalise dist from center so it ranges from 0-1
  float dFromCenter = 2*length(texCoords.xy-vec2(0.5));
  float alpha = 1-pow(dFromCenter,2);
  if(alpha<0.005)
      discard;
  vec3 hsv = vec3(renderOrder,1.0,1.0);
  fragColor = vec4(hsv2rgb(hsv),alpha/2.0);

  //fragColor = vec4(texCoords.xy,0.0,1.0);
}