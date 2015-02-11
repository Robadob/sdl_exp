#version 440

// Inputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 normal;

// Uniforms
layout(location = 1) uniform mat4 modelview_matrix;
layout(location = 2) uniform mat4 projection_matrix;
uniform samplerBuffer location_data_map;

// Outpus
flat out int instanceID;
out vec3 u_normal;


void main(void) {
    // Generate some new position based on the input position and instance Id
    vec4 location_data = texelFetch(location_data_map, int(gl_InstanceID));
    vec3 new_position = in_position.rgb + location_data.rgb;

    gl_Position = projection_matrix * modelview_matrix * vec4(new_position, 1.0);

    // Set output values for subsequent shaders
    instanceID = gl_InstanceID;
    // u_normal = (vec4(in_position, 1.0)).rbg;
    u_normal = (vec4(new_position, 1.0)).rbg;
}
