#ifndef __ViewPort_h__
#define __ViewPort_h__
class Viewport
{
public:
	/*
	House keeping incase someone tries to delete a pointer to the ViewPort base class
	*/
	virtual ~Viewport(){}
	/*
	Returns the pointer to the view frustum of the viewport
	It is intended that this value can be tracked by Shaders over the duration of execution
	@todo Switch to shared_ptr<const glm::mat4> return type
	*/
	virtual const glm::mat4 *getProjectionMatPtr() const = 0;
};
#endif