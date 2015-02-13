#version 440

// Inputs
flat in int instanceID;
in vec3 u_normal;

// Uniforms
uniform samplerBuffer location_data_map;

// Outputs
out vec4 color;

void main()
{

    vec4 location_data = texelFetch(location_data_map, int(instanceID));
    vec3 my_colour = vec3(0.0, 0.0, 0.0);
    if(location_data.a < 1.0){
        my_colour.r = 1.0;
    } else {
        my_colour.b = 1.0;
    }

    // Do some basic shading
    vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));
    vec3 L = normalize(vec3(0,0,0)-u_normal);
    vec3 diffuse = my_colour * max(dot(L, N), 0.0);

    // Set the output value of frag colour
    color = vec4(diffuse.xyz,1.0);
}
