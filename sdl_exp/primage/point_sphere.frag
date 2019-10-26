#version 430
uniform mat4 _viewMat;
uniform mat4 _projectionMat;

in vec2 texCoords;
in vec3 eyeVertex;
flat in vec3 colour;
out vec4 fragColor;

void main()
{
  float sphereRadius=5;
  // calculate eye-space sphere normal from texture coordinates
  vec3 N;
  N.xy = texCoords*2.0-1.0;
  float r2 = dot(N.xy, N.xy);
  if (r2 > 1.0) discard; // kill pixels outside circle
  N.z = sqrt(1.0 - r2);
  N = normalize(N);
  // calculate depth
  vec4 pixelPos = vec4(eyeVertex + N*sphereRadius, 1.0);
  vec4 clipSpacePos = _projectionMat * pixelPos;
  gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
  float diffuse = max(0.0, dot(N, vec3(0,0,1)));

  fragColor = vec4(diffuse*colour, 1.0f);
  //fragColor = vec4(vec3(clipSpacePos.z / clipSpacePos.w),1.0f);
  

}