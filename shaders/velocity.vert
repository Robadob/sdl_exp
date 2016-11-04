#version 430

in vec3 _vertex;

out vec4 vertex;
out vec4 prevVertex;

uniform mat4 _projectionMat;
uniform mat4 _modelViewMat;  // In SceneVelocityShader
uniform mat4 _prevModelViewMat; // In SceneVelocityShader

void main()
{
    vertex  = _projectionMat * _modelViewMat * vec4(_vertex,1.0f);
    prevVertex = _projectionMat * _prevModelViewMat * vec4(_vertex,1.0f);

    gl_Position = _projectionMat * _modelViewMat * vec4(_vertex,1.0f);
}