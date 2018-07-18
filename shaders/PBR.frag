#version 430
/**
 * Physically Based Rendering & Image Based Lighting Fragment Shader
 * Based on the learnopengl.com examples by Joey de Vries
**/

//Prototypes
float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
const uint B_ALBEDO             = 1<<0;
const uint B_ROUGHNESS          = 1<<1;
const uint B_METALLIC           = 1<<2;
const uint B_METALLICROUGHNESS  = 1<<3;
const uint B_NORMAL             = 1<<4;
const uint B_LIGHTMAP           = 1<<5; //Ambient Occlusion
const uint B_EMISSIVE           = 1<<6;
const uint B_DIFFUSE            = 1<<7;
const uint B_SPECULAR           = 1<<8;
const uint B_GLOSSINESS         = 1<<9;
const uint B_SPECULARGLOSSINESS = 1<<10;
const uint B_SHININESS          = 1<<11;
//PBR/IBL Textures
uniform sampler2D t_albedo;
uniform sampler2D t_roughness;
uniform sampler2D t_metallic;//Also handles metallic rough
uniform sampler2D t_normal;
uniform sampler2D t_lightmap;
uniform sampler2D t_emissive;
uniform samplerCube t_diffuse;
uniform samplerCube t_specular;//Also handles specularglossiness
uniform samplerCube t_glossiness;//Also handles shininess
//Vertex attributes
in vec3 eyeVertex;
in vec3 eyeNormal;
in vec2 texCoords;
in mat3 tbnMat;
//Material and Lighting structures
struct PBRmaterial
{
		vec4 albedo;            //Base color (aka diffuse, albedo)
    float roughness;
    float metallic;
    float refractionIndex;	//Unused (padding)
    float alphaCutOff;
    vec3 emissive;          //Unused, Emissive color (light emitted) (disabled to keep struct size down temporary
    uint bitmask;           //bitmask to calculate which textures are available
    //Extension: KHR_materials_pbrSpecularGlossiness
    vec4 diffuse;           //Diffuse reflection color
    vec3 specular;          //Specular reflection color
    float glossiness;       //Glossiness factor
    bool has(uint check) { return (bitmask&check)!=0;}
};
struct LightProperties
{
    vec3 color;                //The radiant colour of the light prior to attenuation
    //4 bytes padding
    vec3 position;             //Position of light, precomputed into eye space 
    //4 bytes padding
    vec3 halfVector;           // Derived: (This is calculated as the vector half way between vector-light and vector-viewer) 
    float spotCosCutoff;       // Derived: cos(Crli) (Valid spotlight range: [1.0,0.0]), negative == pointlight, greater than 1.0 == directional light
    vec3 spotDirection;        // Sdli 
    float spotExponent;        // Srli   
    float constantAttenuation; // K0     
    float linearAttenuation;   // K1   
    float quadraticAttenuation;// K2
    //4 bytes padding
    float attenuation(vec3 L)
    {
        float distSquared = dot(L, L);
        float dist = sqrt(distSquared);
        return 1.0f/
        (
          constantAttenuation
        + linearAttenuation*dist
        + quadraticAttenuation * distSquared
        );
    }
};
const uint MAX_LIGHTS = 50;
const uint MAX_MATERIALS = 50;
uniform uint _materialID;
uniform _materials
{
  PBRmaterial material[MAX_MATERIALS];
};
uniform _lights
{
  uint lightsCount;
  //<12 bytes of padding>
  LightProperties light[MAX_LIGHTS];
};
//Utility functions for deciding whether to pull property from texture or material const
vec4 Albedo()
{
    //return (bitmask&B_ALBEDO)!=0 ? texture(t_albedo, texCoords) : albedo;
    return material[_materialID].has(B_ALBEDO) ? texture(t_albedo, texCoords) : material[_materialID].albedo;
}
vec4 AlbedoSRGB()
{//Gamma corrected from SRGB space (optional)
    vec4 a = Albedo();
    return vec4(pow(a.rgb, vec3(2.2f)), a.a);
}
vec3 Normal()
{//Additionally convert normal to eye space if from texture
    //GLtf2.0 spec says normal material should have scale property
    return normalize(material[_materialID].has(B_NORMAL) ? tbnMat*normalize((2*texture(t_normal, texCoords).xyz)-1) : eyeNormal);
}
float Metallic()
{
    //if((bitmask&B_METALLIC)!=0||(bitmask&B_METALLICROUGHNESS)!=0)
    if(material[_materialID].has(B_METALLIC)||material[_materialID].has(B_METALLICROUGHNESS))
      return texture(t_metallic, texCoords).b;
    return material[_materialID].metallic;
}
float Roughness()
{
    //if((bitmask&B_ROUGHNESS)!=0)
    if(material[_materialID].has(B_ROUGHNESS))
        return texture(t_roughness, texCoords).r;
    //if((bitmask&B_METALLICROUGHNESS)!=0)
    if(material[_materialID].has(B_METALLICROUGHNESS))
        return texture(t_metallic, texCoords).g;
    return material[_materialID].roughness;
}
float AmbientOcclusion()
{
    //return (bitmask&B_LIGHTMAP)!=0 ? texture(t_lightmap, texCoords).r : 0;//AMBIENT OCCLUSION IS MISSING FROM MATERIAL PROPS
    return material[_materialID].has(B_LIGHTMAP) ? texture(t_lightmap, texCoords).r : 0;//AMBIENT OCCLUSION IS MISSING FROM MATERIAL PROPS
}
vec4 Diffuse()
{
    //if((bitmask&B_DIFFUSE)!=0)
    if(material[_materialID].has(B_DIFFUSE))
        return texture(t_diffuse, eyeNormal);//Alpha *may* be present, if so it should be processed according to alphaMode
    return material[_materialID].diffuse;
}
vec3 Specular()
{
    //if((bitmask&B_SPECULAR)!=0||(bitmask&B_SPECULARGLOSSINESS)!=0)
    if(material[_materialID].has(B_SPECULAR)||material[_materialID].has(B_SPECULARGLOSSINESS))
        return texture(t_specular, eyeNormal).rgb;
    return material[_materialID].specular;
}
float Glossiness()
{
    //if((bitmask&B_GLOSSINESS)!=0)
    if(material[_materialID].has(B_GLOSSINESS))
        return texture(t_glossiness, eyeNormal).r;
    //if((bitmask&B_SPECULARGLOSSINESS)!=0)
    if(material[_materialID].has(B_SPECULARGLOSSINESS))
        return texture(t_specular, eyeNormal).a;
    //if((bitmask&B_SHININESS)!=0)
    if(material[_materialID].has(B_SHININESS))
        return 1.0f - (texture(t_glossiness, eyeNormal).r/1000.0f);
    return material[_materialID].glossiness;
}
//Fragment outputs
out vec4 fragColor;
out vec3 fragBright; //Used for Bloom post process

const float PI = 3.14159;
void main()
{
    //Material properties
    const vec4 albedo = Albedo();
    const float metallic = Metallic();
    const float roughness = Roughness();
    const float ambientOcclusion = AmbientOcclusion();
    //Surface normal
    const vec3 N = normalize(Normal()); 
    //Vector from camera to fragment
    const vec3 V = normalize(eyeVertex);
    //Store of the Riemann sum of lighting outgoing radiances
    vec3 Lo = vec3(0);
    //Iterate light sources
    for(uint i = 0;i<lightsCount;i++)
    {
      /**
       * Per light radiance
      **/
      //Vector from light to fragment
      const vec3 L = normalize(light[i].position.xyz - eyeVertex);
      //Half-way vector
      const vec3 H = light[i].halfVector;//normalize(V + L);
      //Attenuation coefficient
      const float attenuation = light[i].attenuation(L);
      //Linear space radiance (gamma correct later)
      const vec3 radiance = vec3(1);//light[i].color;// * attenuation;
      
      /**
       * BRDF (via Cook-Torrance)
      **/
      //Normal Distribution Function (via Trowbridge-Reitz-GGX)
      const float NDF = DistributionGGX(N, H, roughness);
      //Geometry Function (via Smith's Schlick-GGX)
      const float G = GeometrySmith(N, V, L, roughness);
      //Fresnel Equation (via Fresnel-Schlick Approximation)
      vec3 F0 = mix(vec3(material[_materialID].refractionIndex), albedo.rgb, metallic);
      const vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
      
      //Specular radiance
      const vec3 kS = F;
      //Diffuse radiance (energy conserving, account for metals)
      const vec3 kD = vec3(0.5f);//(vec3(1.0f) - kS) * (1.0f - metallic);
      
      const vec3 numerator    = NDF * G * F;
      const float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
      const vec3 specular     = numerator / max(denominator, 0.001f);  
      
      const float NdotL = max(dot(N, L), 0.0);
      //Add to Riemann sum
      Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }
    
    const vec3 ambient = vec3(0.03) * albedo.rgb * ambientOcclusion;
    //HDR color
    vec3 color = ambient + Lo;    
    //Gamma corrected HDR colour
    color = color / (color + vec3(1.0f));
    color = pow(color, vec3(1.0f/2.2f));    
    //Brightness value for bloom calc
    const float brightness = dot(color, vec3(0.2126f, 0.7152f, 0.0722f));    
    //Outputs
    fragColor = vec4(color, albedo.a);    
    fragBright = brightness > 1.0f ? color : vec3(0.0f);
    if(fragColor.a<material[_materialID].alphaCutOff)
      discard;
}
/**
 * Trowbridge-Reitz-GGX:
 * N: Surface normal
 * H: Halfway vector
 * a: Surface roughness
**/
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
  
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
  
    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
  
    return nom / denom;
}
/**
 * Smith's Schlick-GGX
 * N: Surface normal
 * V: View direction
 * L: Light direction
 * k: Surface roughness
**/
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
  
    return ggx1 * ggx2;
}
/**
 * Fresnel-Schlick Approximation
 * cosTheta: dot product of surface normal and view direction
 * F0: normal incidence
**/
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}