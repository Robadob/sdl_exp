#version 430
in vec3 eyeVertex;
in vec3 eyeNormal;
in vec3 eyeLightSource;
in highp vec4 shadowCoord;

uniform sampler2D _shadowMap;

out vec4 fragColor;

uniform vec3 _color;
uniform float zNear = 0.005;
uniform float zFar = 500.0;

void main (void)  
{
    float ambient = 0.3f;
    float diffuse = 0.85f;
	float specular = 0.2f;
    float shiny = 25.0f;
	
	vec4 ambientColor = vec4(_color.xyz/2.0f,1);
	vec4 diffuseColor = vec4(_color,1.0f);
	vec4 specularColor = vec4(_color,1.0f);
	
    vec3 L = normalize(eyeLightSource-eyeVertex);   
    vec3 E = normalize(-eyeVertex); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    vec3 R = normalize(reflect(-L,eyeNormal));  
	
	const float bias = 0.005;
	float visibility = texture(_shadowMap, shadowCoord.xy ).r<shadowCoord.z-bias?0.0f:1.0f;//>shadowCoord.z? 0 : 1.0;

    //calculate Ambient Term:  
    vec4 Iamb = ambientColor * ambient;

    //calculate Diffuse Term:  
    vec4 Idiff = visibility * diffuseColor * diffuse * max(dot(eyeNormal,L), 0.0f);
    Idiff = clamp(Idiff, 0.0f, 1.0f);     

    // calculate Specular Term:
    vec4 Ispec = visibility * specularColor * specular * pow(max(dot(R,E),0.0f),shiny);
    Ispec = clamp(Ispec, 0.0f, 1.0f); 
	
    // write Total Color:    
    fragColor = Iamb + Idiff + Ispec;     
}