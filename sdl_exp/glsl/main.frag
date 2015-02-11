#version 440

flat in int instanceID;
in vec3 u_normal;

out vec4 color;

void main()
{
    float red = 0.1 + ((instanceID % 10) / 9.0);

    vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));
    vec3 L = normalize(vec3(0,0,0)-u_normal);
    vec3 diffuse = vec3(red, red, 0.0) * max(dot(L, N), 0.0);

    color = vec4(diffuse.xyz,1.0);
}
