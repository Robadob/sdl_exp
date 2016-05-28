#version 430

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
    vec3 loc = vec3(5*cos(_tick),5*cos(3.7*_tick),5*sin(_tick));
    distOrigin = length(loc);
    vec3 vertex = loc +_right*_vertex.x+_up*_vertex.y;
    gl_Position =  _projectionMat * modelViewMat * vec4(vertex, 1.0);
}