#version 430
in vec2 texCoords;
in vec3 eyeVertex;
in vec3 eyeNormal;
in vec3 eyeLightSource;
in vec4 shadowCoord;

uniform sampler2D _texture;
uniform sampler2D _shadowMap;

out vec4 fragColor;

uniform mat4 spotlightProjectionMat;
float makeLinearDepth(float depthNorm);
void main (void)  
{    
    float ambient = 0.3f;
    float diffuse = 0.85f;
	float specular = 0.2f;
    float shiny = 25.0f;
	vec3 _color = texture( _texture, vec2(texCoords.x, -texCoords.y)).rgb;
	vec4 ambientColor = vec4(_color.xyz/2.0f,1);
	vec4 diffuseColor = vec4(_color,1.0f);
	vec4 specularColor = vec4(_color,1.0f);
	
    vec3 L = normalize(eyeLightSource-eyeVertex);   
    vec3 E = normalize(-eyeVertex); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    vec3 R = normalize(reflect(-L,eyeNormal));  
	
	/*
	//Basic shadow with bias
	const float bias = 0.005;
	float visibility = texture(_shadowMap, shadowCoord.xy ).r<shadowCoord.z-bias?0.0f:1.0f;
	*/
	
	//Exponential Shadow Maps - Thomas Allen (http://discovery.ucl.ac.uk/10001/1/10001.pdf)
	//Todo: divide values by shadowCoord.w to support projection matrices/spot lights
	//Higher exponent value is closer to the hard-shadow step function
	//Too high and the visibility will overflow
	const float LIGHT_EXPONENT = 80.0f;//Fig 2.0
    //Make linear (same as shadowMap)
	float reciever = makeLinearDepth(shadowCoord.z);
	//cz, corresponding shadow map value
	//(we should really precompute this when creating the shadow map)
	float occluder = exp(LIGHT_EXPONENT*texture(_shadowMap, shadowCoord.xy).r);
	//visibility = e^(-cd)e^(cz), clamped to bounds
	float visibility = clamp(exp(-LIGHT_EXPONENT * reciever) *occluder,0.0,1.0);
		
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
float makeLinearDepth(float depthNorm)
{
    int isOrtho = int(spotlightProjectionMat[3][3]);//This coord is 1 for ortho, 0 for proj
    int isProj = int(-spotlightProjectionMat[2][3]);//This coord is -1 for proj, 0 for ortho
    if(isOrtho==1)//Ortho starts out with linear depth
        return depthNorm;
    //Depth in the normalised range 0 to +1
    //depthNorm
    //Depth in device range -1 to +1
    float depthDevice = (depthNorm * 2.0) - 1.0;
    //Linear depth in range znear - zfar
    //http://stackoverflow.com/a/16597492/1646387
    vec4 unprojected = inverse(spotlightProjectionMat) * vec4(0, 0, depthDevice, 1.0);
    float depthView = -(unprojected.z / unprojected.w);//Negate why? Is it because using ortho matrix
    //Get projection near/far planes from column major matrices
    //http://stackoverflow.com/a/12926655/1646387
    float zNear = (spotlightProjectionMat[3][2] / (spotlightProjectionMat[2][2] - 1.0f));
    float zFar = (spotlightProjectionMat[3][2] / (spotlightProjectionMat[2][2] + 1.0f));
    //Linear depth in range 0-1
	return (depthView-zNear)/(zFar-zNear);
}