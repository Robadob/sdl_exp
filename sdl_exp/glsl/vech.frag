#version 440

// Inputs
flat in int instanceID;
in vec3 u_normal;

in vec3 _N;
in vec3 _v;

// Uniforms
uniform samplerBuffer location_data_map;

// Outputs
out vec4 color;

void main()
{

    vec4 location_data = texelFetch(location_data_map, int(instanceID));
    vec3 my_colour = vec3(0.0, 0.0, 0.0);
    if(location_data.a == 0){
        my_colour.r = 1.0;
        my_colour.g = 1.0;
    } else if (location_data.a == 1 ){
        my_colour.g = 1.0;
        my_colour.b = 1.0;
    } else {
        my_colour.b = 1.0;
        my_colour.r = 1.0;
    }

    // Collection of light attributes temp stored in here.
    vec3 light_position = vec3(0, 0, 0);
    vec4 front_light_ambient = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 front_light_diffuse = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 front_light_specular = vec4(0.1, 0.1, 0.1, 1.0);
    float front_light_shininess = 32;

    // attempt at porting shader from clockwork
    vec3 L = normalize(light_position - _v);
    vec3 E = normalize(-_v); // eye co ords so eye pos is 0,0,0
    vec3 R = normalize(-reflect(L, _N));
    // amb/diff/spec
    vec4 Iamb = front_light_ambient;
    vec4 Idiff = front_light_diffuse * max(dot(_N, L), 0.0);
    Idiff = clamp(Idiff, 0.0, 1.0);
    vec4 Ispec = front_light_specular * pow(max(dot(R, E), 0.0), 0.3 * front_light_shininess);
    Ispec = clamp(Ispec, 0.0, 1.0);
    // Set the final colour
    color = vec4((my_colour/1), 1.0) + Iamb + Idiff + Ispec;

    // End attempt

    

    // Do some basic shading
    // vec3 N  = normalize(cross(dFdx(u_normal), dFdy(u_normal)));
    // vec3 L = normalize(vec3(0,0,0)-u_normal);
    // vec3 diffuse = my_colour * max(dot(L, N), 0.0);

    // // Set the output value of frag colour
    // color = vec4(diffuse.xyz,1.0);
}
