#version 440

// Inputs
flat in int instanceID;
in vec3 u_normal;

// Outputs
out vec4 color;

void main()
{
    // Do some basic shading
    vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));
    vec3 L = normalize(vec3(0,0,0)-u_normal);
    vec3 diffuse = vec3(0.8, 0.8, 0.0) * max(dot(L, N), 0.0);

    // Set the output value of frag colour
    color = vec4(diffuse.xyz,1.0);
}
