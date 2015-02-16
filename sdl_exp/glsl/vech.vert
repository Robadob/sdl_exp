#version 440

// Inputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

// Uniforms
layout(location = 1) uniform mat4 modelview_matrix;
layout(location = 2) uniform mat4 projection_matrix;
// layout(location = 1) uniform mat4 MVP;

uniform samplerBuffer location_data_map;

// Outpus
flat out int instanceID;
out vec3 u_normal;

out vec3 _N;
out vec3 _v;


void main(void) {
    // Generate some new position based on the input position and instance Id
    vec4 location_data = texelFetch(location_data_map, int(gl_InstanceID));
    vec3 new_position = in_position.rgb + location_data.rgb;

    vec3 light_position = vec3(0, 10, 0);

    gl_Position = projection_matrix * modelview_matrix * vec4(new_position, 1.0);

    // Set output values for subsequent shaders
    instanceID = gl_InstanceID;
    // u_normal = (vec4(new_position, 1.0)).rbg;
    u_normal = new_position - light_position;

    _v = vec3(modelview_matrix * vec4(in_position, 1.0));
    mat3 normal_matrix = inverse(transpose(mat3(modelview_matrix))) ; //@todo - this is awful. Should be done once and uniformed
    _N = normalize(normal_matrix * in_normal);
}

