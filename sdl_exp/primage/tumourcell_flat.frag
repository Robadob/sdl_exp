#version 430
struct LightProperties
{
    vec3 ambient;              // Aclarri   
    float spotExponent;        // Srli   
    vec3 diffuse;              // Dcli   
    float PADDING1;            // Crli   (ex spot cutoff, this value is nolonger set internally)                             
    vec3 specular;             // Scli   
    float spotCosCutoff;       // Derived: cos(Crli) (Valid spotlight range: [1.0,0.0]), negative == pointlight, greater than 1.0 == directional light
    vec3 position;             // Ppli   
    float constantAttenuation; // K0   
    vec3 halfVector;           // Derived: Hi  (This is calculated as the vector half way between vector-light and vector-viewer) 
    float linearAttenuation;   // K1   
    vec3 spotDirection;        // Sdli   
    float quadraticAttenuation;// K2  
};
const uint MAX_LIGHTS = 50;

uniform _lights
{
  uint lightsCount;
  //<12 bytes of padding>
  LightProperties light[MAX_LIGHTS];
};

in vec3 eyeVertex;
in vec2 texCoords;
flat in vec3 colour;
out vec4 fragColor;

void main()
{
  //Calculate face normal
  vec3 eyeNormal  = normalize(cross(dFdx(eyeVertex), dFdy(eyeVertex)));

  //Find material colours for each type of light
  vec3 ambient = colour*0.2f;
  vec4 diffuse = vec4(colour, 1.0f);
  vec3 specular = vec3(0);
  
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
      float attenuation;
      float intensity = 1.0f;
      vec3 surfaceToLight;
      //Init light specific values
      if(light[i].spotCosCutoff>1.0f)
      {//Light is directional      
        attenuation = light[i].constantAttenuation;
        surfaceToLight = -light[i].spotDirection;
      }
      else
      {
        surfaceToLight = normalize(light[i].position.xyz - eyeVertex);
        if(light[i].spotCosCutoff>=0.0f)
        {//Spotlight
          float spotCos = dot(surfaceToLight,-light[i].spotDirection);
          //Step works as (spotCos>light[i].spotCosCutoff?0:1)
          //Handle spotExponent
          intensity = step(light[i].spotCosCutoff, spotCos) * pow(spotCos, light[i].spotExponent);
        }
        //Pointlight(or in range spotlight)      
        float dist2 = dot(surfaceToLight, surfaceToLight);
        float dist = sqrt(dist2);
        attenuation = (light[i].constantAttenuation)+(light[i].linearAttenuation*dist)+(light[i].quadraticAttenuation*dist2);
      }
      attenuation = clamp(intensity/attenuation,0.0f,1.0f);
      
      //Process Ambient
      {
        lightAmbient += light[i].ambient * attenuation;
      }
      //Process Diffuse
      {
        float lambertian = max(dot(surfaceToLight,eyeNormal),0.0f);//phong
        lightDiffuse += light[i].diffuse.rgb * lambertian * attenuation;
      }
      
/*       //Process Specular
      if (material[_materialID].shininess == 0 || material[_materialID].shininessStrength == 0)
        continue;//Skip if no shiny
      {
        vec3 reflectDir = reflect(-surfaceToLight, eyeNormal);
        float specAngle = max(dot(reflectDir, normalize(-eyeVertex)), 0.0);
        float spec = clamp(pow(specAngle, material[_materialID].shininess/4.0), 0.0f, 1.0f); 
        lightSpecular += light[i].specular * spec * attenuation;
      } */
    } 
    
    //Export lights
    ambient *= lightAmbient;
    diffuse *= vec4(lightDiffuse, 1.0f);
    specular *= lightSpecular;   
  }

  vec3 color = clamp(ambient + diffuse.rgb + specular,0,1);

  //fragColor = vec4(color, min(diffuse.a, material[_materialID].opacity));//What to do with opac?
  fragColor = vec4(color, 1.0f);//What to do with opac?
  
  //Discard full alpha fragments (removes requirement of back to front render/glblend)
  if(fragColor.a<=0.0f)
    discard;
}