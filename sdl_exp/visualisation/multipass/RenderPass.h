#ifndef __RenderPass_h__
#define __RenderPass_h__
#include "../interface/FBuffer.h"
#include <memory>
#include <glm/glm.hpp>

/**
 * Base class that binds the named FrameBuffer before calling the subclasses render() to trigger rendering.
 */
class RenderPass
{
public:
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka back buffer)
     * The resolution of the backbuffer is controlled by the viewport
     * @param clearColor The colour to clear the backBuffer with prior to render, each component is in the range 0-1
     * @param doClear Whether to clear prior to render
     */
	RenderPass(glm::vec3 clearColor = glm::vec3(0), bool doClear = true);
    /**
     * Creates a Renderpass which renders to a custom FrameBuffer
     */
	RenderPass(std::shared_ptr<FBuffer> fb);
    /**
     * Allow subclasses to be destroyed properly by RenderPass ptrs
     */
    virtual ~RenderPass()
    { }
	std::shared_ptr<FBuffer> getFrameBuffer() { return fb; }
    /**
     * Binds the contained FrameBuffer and then triggers render()
     * If the FrameBuffer is incomplete (invalid), the render will be skipped and the previous framebuffer bound
	 * @note This method is called by the scene in order of RenderPass priority
	 */
	virtual void executeRender();
    /**
     * Triggered when the viewport resizes, such that attatched textures can be auto resized
     */
	virtual void resize(int width, int height);
private:
	const std::shared_ptr<FBuffer> fb;
protected:
    /**
     * Called by executeRender() after framebuffer has been bound
     */
    virtual void render() = 0;
};
#endif //__RenderPass_h__