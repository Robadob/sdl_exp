#version 430

in vec3 eyeVertex;
in vec3 eyeNormal;

uniform vec4 _color;

out vec4 fragColor;

void main()
{
  //Find material colours for each type of light
  vec3 ambient = _color.rgb;
  vec4 diffuse = _color;
  vec3 specular = vec3(1);
  
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
    vec3 reflectDir = reflect(-surfaceToLight, eyeNormal);
    float specAngle = max(dot(reflectDir, surfaceToLight), 0.0);
    float shininess = 1.0f;
    float spec = clamp(pow(specAngle, shininess), 0.0f, 1.0f); 
    specular *= 0.2f * spec;

  vec3 color = clamp(ambient + diffuse.rgb + specular,0,1);

  fragColor = vec4(color, diffuse.a);
}