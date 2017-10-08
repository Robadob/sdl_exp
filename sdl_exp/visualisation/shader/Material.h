#ifndef __Material_h__
#define __Material_h__

#include <glm/glm.hpp>

/**
 * Only values requested by the shader are used, others can be ignored
 */
struct Material
{
	glm::vec3 diffuse;
	float opacity;
	glm::vec3 specular;
	float shininess;//Exponent?
	glm::vec3 ambient;
	float shininessStrength;
	glm::vec3 emissive;
	float refractionIndex;
	glm::vec3 transparent;
};
#endif //__Material_h__