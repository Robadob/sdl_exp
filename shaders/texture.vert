#version 120

varying vec2 _coords;

void main()
{
  _coords = gl_Color.xy;//gl_TexCoord[0].xyz;
  ////pass gl_Vertex to frag shader to calculate face normal
  //u_normal = (_modelViewMat * vec4(_vertex,1.0f)).rgb;
  //apply model view proj
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}