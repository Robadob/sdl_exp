#version 430
//gl_InstanceID //attribute in uint agent_index;

in vec3 _vertex;
out vec3 u_normal;

uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;

uniform samplerBuffer _texBuf;

uniform vec3 _color;
out vec3 o_color;

void main(){
  //Grab model offset from textures
  vec3 loc_data = texelFetch(_texBuf, gl_InstanceID).xyz;//vec3(gl_InstanceID,0,0);//
  //Output vert loc to be interpolated for shader to calc norm
  vec3 t_position = _vertex + loc_data;
  u_normal = vec4(_modelViewMat * vec4(t_position,1.0)).xyz;
  gl_Position = _projectionMat * _modelViewMat * vec4(t_position,1.0);
  o_color = _color;
}