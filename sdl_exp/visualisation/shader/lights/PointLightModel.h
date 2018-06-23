#ifndef __PointLightModel_h__
#define __PointLightModel_h__
#include <glm/glm.hpp>
#include "../../util/GLcheck.h"
#include "../Shaders.h"
#include "../../interface/Renderable.h"

/**
* Provides a basic sphere model for representing a PointLight within a scene
* Intended for debugging, rather than graphical fidelity
*/
class PointLightModel : public Renderable
{
	static const char NFF_SPHERE[];
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
	PointLightModel();
	~PointLightModel();
	/**
	 * Sets the diameter of point light
	 * Default value is 1.0
	 */
	void setScale(const float &scale){ this->scale = scale; }
	/**
	 * @param location Source of the light
	 * @param color Color to render the light (probably diffuse)
	 */
	virtual void render(const glm::vec3 &location, const glm::vec3 &color) const;
	void setViewMatPtr(const glm::mat4 *viewMat) override;
	void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
	void reload() override;
	/**
	 * Redundant
	 */
	void setLightsBuffer(GLuint bufferBindingPoint) override;
};

#endif //__PointLightModel_h__