#version 430
const uint B_NONE         = 1<<0;
const uint B_AMBIENT      = 1<<1;
const uint B_DIFFUSE      = 1<<2;
const uint B_SPECULAR     = 1<<3;
const uint B_EMISSIVE     = 1<<4;
const uint B_HEIGHT       = 1<<5;
const uint B_NORMAL       = 1<<6;
const uint B_SHININESS    = 1<<7;
const uint B_OPACITY      = 1<<8;
const uint B_DISPLACEMENT = 1<<9;
const uint B_LIGHT        = 1<<10;
const uint B_REFLECTION   = 1<<11;
const uint B_UNKNOWN      = 1<<12;
struct MaterialProperties
{
    vec3 ambient;           //Ambient color
    float opacity;
    vec3 diffuse;           //Diffuse color
    float shininess;
    vec3 specular;          //Specular color
    float shininessStrength;
    vec3 emissive;          //Emissive color (light emitted)
    float refractionIndex;
    vec3 transparent;       //Transparent color, multiplied with translucent light to construct final color
    uint bitmask;
    bool has(uint check) { return (bitmask&check)!=0;}
};
struct LightProperties
{
    vec3 ambient;              // Aclarri   
    float spotExponent;        // Srli   
    vec3 diffuse;              // Dcli   
    float PADDING1;            // Crli   (ex spot cutoff, this value is nolonger set internally)                             
    vec3 specular;             // Scli   
    float spotCosCutoff;       // Derived: cos(Crli) (range: [1.0,0.0],-1.0)   
    vec3 position;             // Ppli   
    float constantAttenuation; // K0   
    vec3 halfVector;           // Derived: Hi  (This is calculated as the vector half way between vector-light and vector-viewer) 
    float linearAttenuation;   // K1   
    vec3 spotDirection;        // Sdli   
    float quadraticAttenuation;// K2  
};
const uint MAX_LIGHTS = 50;
const uint MAX_MATERIALS = 50;

uniform uint _materialID;
uniform _materials
{
  MaterialProperties material[MAX_MATERIALS];
};
uniform _lights
{
  uint lightsCount;
  //<12 bytes of padding>
  LightProperties light[MAX_LIGHTS];
};

uniform sampler2D t_ambient;
uniform sampler2D t_diffuse;
uniform sampler2D t_specular;

in vec3 eyeVertex;//Vertex in world coordinates
in vec3 eyeNormal;
in vec2 texCoords;

out vec4 fragColor;

float makeVisibility();
void main()  
{  
  //Find material colours for each type of light
  vec3 ambient = material[_materialID].has(B_AMBIENT) ? texture(t_ambient, texCoords).rgb : material[_materialID].ambient;
  vec4 diffuse = material[_materialID].has(B_DIFFUSE) ? texture(t_diffuse, texCoords) : vec4(material[_materialID].diffuse, 1.0f);
  vec3 specular = material[_materialID].has(B_SPECULAR) ? texture(t_specular, texCoords).rgb : material[_materialID].specular;
  
  //Apply shadow visibility (1 shadow map shadows all lights, but meh)
  float visibility = makeVisibility();
  diffuse *= visibility;
  specular *= visibility;
  
  //No lights, so render full bright
  if(lightsCount>0)
  {
    //Init colours to build light values in
    vec3 lightAmbient = vec3(0);
    vec3 lightDiffuse = vec3(0);
    vec3 lightSpecular = vec3(0);
    
    //Init general values used in light computation    
    for(uint i = 0;i<lightsCount;i++)
    {
      //Init light specific values      
      vec3 surfaceToLight = normalize(light[i].position.xyz - eyeVertex);
      if(light[i].spotCosCutoff>=0&&light[i].spotCosCutoff<=1.0f)
      {//Spotlight
        if(dot(surfaceToLight,light[i].spotDirection)>=light[i].spotCosCutoff)
        {//Skip spotlight if we are out of it's cone
          continue;
        }
      }
      float dist2 = dot(surfaceToLight, surfaceToLight);
      float dist = sqrt(dist2);
      float attenuation = clamp(1/((light[i].constantAttenuation)+(light[i].linearAttenuation*dist)+(light[i].quadraticAttenuation*dist2)),0.0f,1.0f);
      
      //Process Ambient
      {
        lightAmbient += light[i].ambient * attenuation;
      }
      //Process Diffuse
      {
        float lambertian = max(dot(surfaceToLight,eyeNormal),0.0f);//phong
        lightDiffuse += light[i].diffuse.rgb * lambertian * attenuation;
      }
      
      //Process Specular
      if (material[_materialID].shininess == 0 || material[_materialID].shininessStrength == 0)
        continue;//Skip if no shiny
      {
        vec3 reflectDir = reflect(-surfaceToLight, eyeNormal);
        float specAngle = max(dot(reflectDir, normalize(-eyeVertex)), 0.0);
        float spec = clamp(pow(specAngle, material[_materialID].shininess/4.0), 0.0f, 1.0f); 
        lightSpecular += light[i].specular * spec * attenuation;
      }
    } 
    
    //Export lights
    ambient *= lightAmbient;
    diffuse *= vec4(lightDiffuse, 1.0f);
    specular *= lightSpecular;
  }   

  vec3 color = clamp(ambient + diffuse.rgb + specular,0,1);

  fragColor = vec4(color, min(diffuse.a, material[_materialID].opacity));//What to do with opac?
  
  //Discard full alpha fragments (removes requirement of back to front render/glblend)
  if(fragColor.a<=0.0f)
    discard;  
}
/**
 * Single light source shadow map visibility code
**/
in highp vec4 shadowCoord;
uniform sampler2D _shadowMap;
uniform mat4 spotlightProjectionMat;

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
float makeVisibility()
{
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
	return clamp(exp(-LIGHT_EXPONENT * reciever) *occluder,0.0,1.0);
}