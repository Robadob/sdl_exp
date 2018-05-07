#ifndef __Renderable_h__
#define __Renderable_h__

#include "Reloadable.h"
#include <glm/mat4x4.hpp>

#include "Viewport.h"
#include "../Camera.h"

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
		setProjectionMatPtr(visualisation->getFrustrumPtr());
	}
	/**
	* Sets the pointer from which the Model matrix should be loaded from
	* @param modelMat A pointer to the modelMatrix to be tracked
	* @note This pointer is likely provided by the Visualisation object
	*/
	virtual void setModelMatPtr(const glm::mat4 *modelMat) = 0;
	/**
	* Overrides the model matrix (and all dependent matrices) until useProgram() is next called
	* @param modelMat Pointer to the overriding modelMat
	*/
	virtual void overrideModelMat(const glm::mat4 *modelMat) = 0;

};

#endif //__Renderable_h__