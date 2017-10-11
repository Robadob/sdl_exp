#ifndef __RenderPass_h__
#define __RenderPass_h__
#include "../interface/FBuffer.h"
#include <memory>
#include <glm/glm.hpp>

/**
 * Base class that binds the named FrameBuffer before calling the subclasses render() to trigger rendering.
 * @note 'RenderPass::RenderPass' : ambiguous call to overloaded function while 
 * trying to match the argument list '(std::shared_ptr<BackBuffer>)
 * I'm hoping this will be fixed with VS2015, until then cast to std::shared_ptr<FBuffer>
 * Merging the constructors so they take the form vec3, bool seems to also work
 */
class RenderPass
{
public:
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka BackBuffer)
     * The resolution of the backbuffer is controlled by the viewport
     * @param doClear Whether to clear prior to render
     * @note The static backbuffer color will be used
     */
    explicit RenderPass(const bool &doClear = true);
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka BackBuffer)
     * The resolution of the backbuffer is controlled by the viewport
     * @param clearColor The colour to clear the backBuffer with prior to render, each component is in the range 0-1
     */
    RenderPass(const glm::vec3 &clearColor);
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
	virtual void resize(const glm::uvec2 &dims);
private:
	const std::shared_ptr<FBuffer> fb;
protected:
    /**
     * Called by executeRender() after framebuffer has been bound
     */
    virtual void render() = 0;
};
#endif //__RenderPass_h__