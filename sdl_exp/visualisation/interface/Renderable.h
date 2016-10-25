#ifndef __Renderable_h__
#define __Renderable_h__

#include "Reloadable.h"
#include <glm/mat4x4.hpp>

#include "Viewport.h";
#include "../Camera.h";

/**
 * Represents things which hold shaders (and can be rendered)
 * @note Entities are the target audience
 */
class Renderable : public Reloadable
{
public:
	/**
	 * Binds the provided modelview matrix to the internal shader
	 * @param modelViewMat Ptr to modelview matrix
	 * @note This is normally found within the Camera object
	 */
	virtual void setModelViewMatPtr(glm::mat4 const *modelViewMat)=0;
	/**
	 * Binds the provided projection matrix to the internal shader
	 * @param projectionMat Ptr to model view matrix
	 * @note This is normally found within the Viewport object
	 */
	virtual void setProjectionMatPtr(glm::mat4 const *projectionMat) = 0;
	/**
	* Binds the provided modelview matrix to the internal shader
	* @param camera Ptr to modelview matrix
	* @note This is normally found within the Camera object
	* @note This method is overriden by Skybox as it required modelview sans translation
	*/
	void setModelViewMatPtr(const Camera *camera)
	{
		setModelViewMatPtr(camera->getViewMatPtr());
	}
	/**
	* Binds the provided projection matrix to the internal shader
	* @param visualisation Ptr to model view matrix
	* @note Convenience wrapper
	*/
	virtual void setProjectionMatPtr(const Viewport *visualisation) final
	{
		setProjectionMatPtr(visualisation->getFrustrumPtr());
	}

};

#endif //__Renderable_h__