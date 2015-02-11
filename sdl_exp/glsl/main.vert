#version 440

// Inputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 normal;

// Uniforms
layout(location = 1) uniform mat4 modelview_matrix;
layout(location = 2) uniform mat4 projection_matrix;

// Outpus
flat out int instanceID;
out vec3 u_normal;


void main(void) {
    // Generate some new position based on the input position and instance Id
    vec3 new_position = in_position;
    new_position.x += (gl_InstanceID % 10) * 3.0;
    new_position.z += (gl_InstanceID / 10) * 2.0;
    gl_Position = projection_matrix * modelview_matrix * vec4(new_position, 1.0);

    // Set output values for subsequent shaders
    instanceID = gl_InstanceID;
    u_normal = (vec4(in_position, 1.0)).rbg;
}
