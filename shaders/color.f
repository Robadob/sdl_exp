#version 430
 
in vec4 o_color;
in vec3 o_normal;
in vec3 o_vertex;
 
out vec4 fragColor;
 
void main()
{     
    float ambient = 0.6;
    float diffuse = 0.4;
    float shiny = 0.01;

    vec3 L = normalize(vec3(0)-o_vertex);   
    vec3 E = normalize(vec3(0)); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    vec3 R = normalize(-reflect(L,o_normal));  

    //calculate Ambient Term:  
    vec4 Iamb = o_color * ambient;

    //calculate Diffuse Term:  
    vec4 Idiff = o_color *diffuse* max(dot(o_normal,L), 0.0);
    Idiff = clamp(Idiff, 0.0, 1.0);     

    // calculate Specular Term:
    vec4 Ispec = o_color * pow(max(dot(R,E),0.0),0.3*shiny);
    Ispec = clamp(Ispec, 0.0, 1.0); 
    fragColor = Iamb + Idiff + Ispec;
}