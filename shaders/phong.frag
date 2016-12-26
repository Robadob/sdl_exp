#version 430
//https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
in vec3 eyeVertex;
in vec3 eyeNormal;
in vec3 eyeLightSource;

out vec4 fragColor;

uniform vec3 _color;

void main (void)  
{  
    float ambient = 0.3f;
    float diffuse = 0.85f;
	float specular = 0.2f;
    float shiny = 25.0f;
	
	vec4 ambientColor = vec4(_color.xyz/2.0f,1);
	vec4 diffuseColor = vec4(_color,1.0f);
	vec4 specularColor = vec4(_color,1.0f);
	
	vec4 col2 = vec4(0.5,0,0,1);
    //vec3 L = normalize(gl_LightSource[0].position.xyz-v);   
    vec3 L = normalize(eyeLightSource-eyeVertex);   
    vec3 E = normalize(-eyeVertex); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    vec3 R = normalize(reflect(-L,eyeNormal));  

    //calculate Ambient Term:  
    vec4 Iamb = ambientColor * ambient;

    //calculate Diffuse Term:  
    vec4 Idiff = diffuseColor * diffuse * max(dot(eyeNormal,L), 0.0f);
    Idiff = clamp(Idiff, 0.0f, 1.0f);     

    // calculate Specular Term:
    vec4 Ispec = specularColor * specular * pow(max(dot(R,E),0.0f),shiny);
    Ispec = clamp(Ispec, 0.0f, 1.0f); 
	
    // write Total Color:  
    fragColor = Iamb + Idiff + Ispec;     
}