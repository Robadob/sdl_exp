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
    float spotCutoff;          // Crli                              
    // (range: [0.0,90.0], 180.0)   
    vec3 specular;             // Scli   
    float spotCosCutoff;       // Derived: cos(Crli)//Should be in range 0-90)               
    // (range: [1.0,0.0],-1.0)   
    vec3 position;             // Ppli   
    float constantAttenuation; // K0   
    vec3 halfVector;           // Derived: Hi  (This is calculated as the vector half way between vector-light and vector-viewer) 
    float linearAttenuation;   // K1   
    vec3 spotDirection;        // Sdli   
    float quadraticAttenuation;// K2  
};
const unsigned int MAX_LIGHTS = 50;
const unsigned int MAX_MATERIALS = 50;

in vec3 eyeVertex;//Vertex in world coordinates
in vec3 eyeNormal;
in vec2 texCoords;

uniform mat4 _viewMat;//Transform lights into eye space (eventually move this transform to CPU)

out vec4 fragColor;
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


void main()
{
  //Find material colours for each type of light
  vec3 ambient = material[_materialID].has(B_AMBIENT) ? texture(t_ambient, texCoords).rgb : material[_materialID].ambient;
  vec3 diffuse = material[_materialID].has(B_DIFFUSE) ? texture(t_diffuse, texCoords).rgb : material[_materialID].diffuse;
  vec3 specular = material[_materialID].has(B_SPECULAR) ? texture(t_specular, texCoords).rgb : material[_materialID].specular;
  
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
        lightDiffuse += light[i].diffuse * lambertian * attenuation;
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
    diffuse *= lightDiffuse;
    specular *= lightSpecular;   
  }

  vec3 color = ambient + diffuse + specular;

  fragColor = vec4(color, 1.0f);//What to do with opac?
}