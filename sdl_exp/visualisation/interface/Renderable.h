#ifndef __Renderable_h__
#define __Renderable_h__

#include "Reloadable.h"
#include <glm/mat4x4.hpp>

#include "Viewport.h"
#include "../Camera.h"
#include <memory>
#include "../texture/TextureCubeMap.h"

class LightsBuffer;

/**
 * Represents things which hold shaders (and can be rendered)
 * @note Entities are the target audience
 */
class Renderable : public Reloadable
{
public:
	/**
	 * Binds the provided modelview matrix to the internal shader
	 * @param viewMat Ptr to modelview matrix
	 * @note This is normally found within the Camera object
	 */
    virtual void setViewMatPtr(glm::mat4 const *viewMat) = 0;
	/**
	 * Binds the provided projection matrix to the internal shader
	 * @param projectionMat Ptr to model view matrix
	 * @note This is normally found within the Viewport object
	 */
	virtual void setProjectionMatPtr(glm::mat4 const *projectionMat) = 0;
	/**
	 * Binds the provided view matrix to the internal shader
	 * @param camera Ptr to view matrix
	 * @note This is normally found within the Camera object
	 * @note This method is overriden by Skybox as it required modelview sans translation
	 */
	virtual void setViewMatPtr(const Camera *camera)
	{
		setViewMatPtr(camera->getViewMatPtr());
	}
	/**
	 * Binds the provided projection matrix to the internal shader
	 * @param visualisation Ptr to model view matrix
	 * @note Convenience wrapper
	 */
	virtual void setProjectionMatPtr(const Viewport *visualisation) final
	{
		setProjectionMatPtr(visualisation->getProjectionMatPtr());
	}
	virtual void setLightsBuffer(GLuint bufferBindingPoint) = 0;
	virtual void setLightsBuffer(std::shared_ptr<LightsBuffer> buffer);
	
};
class RenderableAdv : public Renderable
{
public:
	/**
	* Sets a cube map texture to be used for providing reflections
	* If static, likely the skybox texture
	* @param cubeMap The cube map texture to be used to provide reflections
	*/
	virtual void setEnvironmentMap(std::shared_ptr<const TextureCubeMap> cubeMap) = 0;

	bool visible() const { return mVisible; }
	void visible(const bool &v) { mVisible = v; }
	virtual glm::vec3 getLocation() const = 0;
	virtual void render(const unsigned int &shaderIndex) = 0;
protected:
	bool mVisible = true;
};

#endif //__Renderable_h__