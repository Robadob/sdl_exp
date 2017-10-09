#ifndef __ViewPort_h__
#define __ViewPort_h__

#include <memory>
#include "../HUD.h"

class Viewport
{
public:
	/*
	House keeping incase someone tries to delete a pointer to the ViewPort base class
	*/
	virtual ~Viewport(){}
	/**
	* @return The current viewport dimensions
	*/
	virtual const glm::uvec2& getWindowDims() const = 0;
	/*
	Returns the pointer to the view frustum of the viewport
	It is intended that this value can be tracked by Shaders over the duration of execution
	@todo Switch to shared_ptr<const glm::mat4> return type
	*/
	virtual const glm::mat4 *getProjMatPtr()const = 0;
	virtual const glm::mat4 getProjMat()const = 0;
};
class Camera;
class ViewportExt : public Viewport
{
public:
	virtual ~ViewportExt(){}
	virtual std::shared_ptr<const Camera> getCamera() const = 0;
	virtual std::weak_ptr<HUD> getHUD() = 0;
	/**
	 * @return The current window title
	 */
	virtual const char *getWindowTitle() const = 0;
	/**
	 * Sets the window title
	 * @param windowTitle Desired title of the window
	 */
	virtual void setWindowTitle(const char *windowTitle) = 0;
};
#endif