#ifndef __Light_h__
#define __Light_h__

#include <glm/glm.hpp>

/**
* Only values requested by the shader are used, others can be ignored
*/
struct Light
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 position;
	glm::vec4 halfVector;
	glm::vec3 spotDirection;
	float spotExponent;
	float spotCutoff;//Treated as radians to auto set spotCosCutoff
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};
#endif //__Light_h__