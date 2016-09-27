#version 430
//http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/#solution-2--the-3d-way
in vec3 _vertex;

in vec3 _texCoords;
out vec3 texCoords;
out float distOrigin;
uniform float _tick;
uniform mat4 _modelViewMat;
uniform mat4 _projectionMat;
uniform vec3 _up;
uniform vec3 _right;
void main () {
    
    texCoords = _texCoords;
    mat4 modelViewMat = _modelViewMat;
    float tick = _tick + gl_InstanceID/1.41421;
    float tick2 = _tick + gl_InstanceID/0.3812;
    vec3 loc = vec3(5*cos(tick),5*cos(3.7*tick),gl_InstanceID);//5*sin(tick2));
    distOrigin = int(_tick*200 + gl_InstanceID*15)%360;//length(loc);
    vec3 vertex = loc +_right*_vertex.x+_up*_vertex.y;
    gl_Position =  _projectionMat * modelViewMat * vec4(vertex, 1.0);
}