#version 120
//https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
varying vec3 N;
varying vec3 v;    
void main (void)  
{  
   float ambient = 0.6;
   float diffuse = 0.4;
   float shiny = 0.01;
   
   vec3 L = normalize(vec3(0)-v);   
   vec3 E = normalize(vec3(0)); // we are in Eye Coordinates, so EyePos is (0,0,0)  
   vec3 R = normalize(-reflect(L,N));  
 
   //calculate Ambient Term:  
   vec4 Iamb = gl_Color * ambient;

   //calculate Diffuse Term:  
   vec4 Idiff = gl_Color *diffuse* max(dot(N,L), 0.0);
   Idiff = clamp(Idiff, 0.0, 1.0);     
   
   // calculate Specular Term:
   vec4 Ispec = gl_Color 
                * pow(max(dot(R,E),0.0),0.3*shiny);
   Ispec = clamp(Ispec, 0.0, 1.0); 
   // write Total Color:  
   gl_FragColor = Iamb + Idiff + Ispec;     
}
      