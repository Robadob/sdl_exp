#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewP;
uniform mat4 _modelViewMat;
uniform mat4 _viewMat;
uniform mat4 _projectionMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

uniform samplerBuffer _texBufX;
uniform samplerBuffer _texBufY;
uniform samplerBuffer _texBufZ;
uniform samplerBuffer _texBufP53;
uniform int instanceOffset;

out vec3 eyeVertex;
out vec3 eyeUNormal;
out vec2 texCoords;
flat out vec3 colour;

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

void main()
{
  //Grab model offset from texture array
  vec3 loc_data;
  loc_data.x = texelFetch(_texBufX, instanceOffset+gl_InstanceID).x;
  loc_data.y = texelFetch(_texBufY, instanceOffset+gl_InstanceID).x;
  loc_data.z = texelFetch(_texBufZ, instanceOffset+gl_InstanceID).x;
  float p53 = texelFetch(_texBufP53, instanceOffset+gl_InstanceID).x;
  //colour
  colour = hsv2rgb(vec3(0,0.75,1.0));
  vec4 modelVert = _modelMat * vec4(_vertex, 1.0f);
  modelVert.xyz = modelVert.xyz + loc_data;
  gl_Position = _projectionMat * _viewMat * modelVert;

  eyeVertex = (_viewMat * vec4(_vertex+loc_data, 1.0f)).rgb;
  texCoords = _texCoords;
}