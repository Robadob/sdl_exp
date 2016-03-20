#version 120
//https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
varying vec3 N;
varying vec3 v;    
void main (void)  
{  
    float ambient = 0.6f;
    float diffuse = 0.4f;
    float shiny = 0.01f;

    vec3 L = normalize(vec3(0)-v);   
    vec3 E = normalize(vec3(0)); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    vec3 R = normalize(-reflect(L,N));  

    //calculate Ambient Term:  
    vec4 Iamb = gl_Color * ambient;

    //calculate Diffuse Term:  
    vec4 Idiff = gl_Color *diffuse* max(dot(N,L), 0.0f);
    Idiff = clamp(Idiff, 0.0f, 1.0f);     

    // calculate Specular Term:
    vec4 Ispec = gl_Color 
                * pow(max(dot(R,E),0.0f),0.3f*shiny);
    Ispec = clamp(Ispec, 0.0f, 1.0f); 
    // write Total Color:  
    gl_FragColor = Iamb + Idiff + Ispec;     
}
      