#version 440

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;


layout(location = 1) uniform mat4 mvMatrix;
layout(location = 2) uniform mat4 mvpMatrix;

flat out int instanceID;
out vec3 u_normal;


void main(void) {
    vec3 newpos = vec3(inPosition.x + (gl_InstanceID % 10) * 2, inPosition.y , inPosition.z + gl_InstanceID /10 * 2);

	gl_Position = mvpMatrix * mvMatrix * vec4(newpos, 1.0);
    instanceID = gl_InstanceID;
    u_normal = (vec4(inPosition, 1.0)).rbg;
}
