#ifndef __SpotLightModel_h__
#define __SpotLightModel_h__
#include <glm/glm.hpp>
#include "../../util/GLcheck.h"
#include "../Shaders.h"
#include "../../interface/Renderable.h"

/**
 * Provides a basic cone model for representing a SpotLight within a scene
 * Intended for debugging, rather than graphical fidelity
 * Angle of cone will match that of spotlight cos cut off
 */
class SpotLightModel : public Renderable
{
	static const char NFF_CONE[];
	glm::vec3 *vertices;
	glm::vec3 *normals;
	unsigned int *faces;
	unsigned int vCount;
	unsigned int fCount;
	GLuint vbo;
	GLuint ibo;
	std::unique_ptr<Shaders> shader;
	float scale = 1.0f;
	glm::mat4 rootTransform;
public:
	SpotLightModel();
	~SpotLightModel();
	/**
	 * Sets the scale of point light
	 * Default value is 1.0 (which leads to 5.0 len cone)
	 */
	void setScale(const float &scale){ this->scale = scale; }
	/**
	 * @param location Source of the light
	 * @param direction Direction of the spotlight
	 * @param cosCutOff Cut off angle of the spotlight (in radians)
	 * @param color Color to render the light (probably diffuse)
	 */
	void render(const glm::vec3 &location, const glm::vec3 &direction, const float &cosCutOff, const glm::vec3 &color) const;
	void setViewMatPtr(const glm::mat4 *viewMat) override;
	void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
	void reload() override;
	/**
	 * Redundant
	 */
	void setLightsBuffer(const GLuint &bufferBindingPoint) override;
};

#endif //__SpotLightModel_h__