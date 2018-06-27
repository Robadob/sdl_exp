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
    float shininessStrength;//Multiplied by Specular color
		//vec3 emissive;        //Unused, Emissive color (light emitted) (disabled to keep struct size down until implemented)
		float reflectivity;     //Reflection factor
		uint padding1;
		uint padding2;
    float refractionIndex;
    vec3 transparent;       //Transparent color, multiplied with translucent light to construct final color
    uint bitmask;
    bool has(uint check) { return (bitmask&check)!=0;}
};
const uint MAX_MATERIALS = 50;

uniform uint _materialID;
uniform _materials
{
  MaterialProperties material[MAX_MATERIALS];
};

uniform sampler2D t_ambient;
uniform sampler2D t_diffuse;
uniform sampler2D t_specular;
uniform samplerCube t_reflection;

in vec3 eyeVertex;
in vec3 eyeNormal;
in vec2 texCoords;

out vec4 fragColor;

void main()
{
  //Find material colours for each type of light
  vec3 ambient = material[_materialID].has(B_AMBIENT) ? texture(t_ambient, texCoords).rgb : material[_materialID].ambient;
  vec4 diffuse = material[_materialID].has(B_DIFFUSE) ? texture(t_diffuse, texCoords) : vec4(material[_materialID].diffuse, 1.0f);
  vec3 specular = material[_materialID].has(B_SPECULAR) ? texture(t_specular, texCoords).rgb : material[_materialID].specular;
  
  //Scale specular according to shininessStrength
  specular*=material[_materialID].shininessStrength;
  
  //No lights, so render full bright pretending eye is the light source
  vec3 surfaceToLight = normalize(-eyeVertex);
  
  //Process Ambient
  {//Ambient component: 0.2f
    ambient *= 0.2f;
  }
  //Process Diffuse
  {//Diffuse component: 0.85f
    float lambertian = max(dot(surfaceToLight,eyeNormal),0.0f);//phong
    diffuse *=  vec4(vec3(lambertian * 0.85f), 1.0f);
  }
  
  //Process Specular
  if (material[_materialID].shininess != 0)
  {//Specular component: 0.2f
    vec3 reflectDir = reflect(-surfaceToLight, eyeNormal);
    float specAngle = max(dot(reflectDir, surfaceToLight), 0.0);
    float spec = clamp(pow(specAngle, material[_materialID].shininess), 0.0f, 1.0f); 
    specular *= 0.2f * spec;
  }

  vec3 color = clamp(ambient + diffuse.rgb + specular,0,1);

  fragColor = vec4(color, min(diffuse.a, material[_materialID].opacity));//What to do with opac?
  
  //Discard full alpha fragments (removes requirement of back to front render/glblend)
  if(fragColor.a<=0.0f)
    discard;
}