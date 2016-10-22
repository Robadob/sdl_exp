#ifndef __RenderPass_h__
#define __RenderPass_h__
#include "FrameBuffer.h"
#include <memory>

/**
 * Base class that binds the named FrameBuffer before calling the subclasses render() to trigger rendering.
 */
class RenderPass
{
public:
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka back buffer)
     * This constructor creates one which has dimensions as a proportion of the viewport resolution
     */
    RenderPass(float scale=1, glm::vec3 clearColor=glm::vec3(0),bool doClear=true)
        : fb()
        , clearColor(clearColor)
        , doClear(doClear)
        , scale(scale)
    { }
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka back buffer)
     * This constructor creates one which has fixed dimensions, and will not scale with
     * @note This may not be required
     */
    RenderPass(glm::vec2 dimensions, glm::vec3 clearColor = glm::vec3(0), bool doClear = true)
        : fb()
        , clearColor(clearColor)
        , doClear(doClear)
        , scale(0)
        , dimensions(dimensions)
    { }
    /**
     * Creates a Renderpass which renders to a custom FrameBuffer
     * This constructor creates one which has dimensions as a proportion of the viewport resolution
     */
    RenderPass(std::shared_ptr<FrameBuffer> fb, float scale = 1.0f, glm::vec3 clearColor = glm::vec3(0), bool doClear = true)
        : fb(fb)
        , clearColor(clearColor)
        , doClear(doClear)
        , scale(scale)
    { }
    /**
     * Creates a Renderpass which renders to a custom FrameBuffer
     * This constructor creates one which has fixed dimensions, and will not scale with
     */
    RenderPass(std::shared_ptr<FrameBuffer> fb, glm::vec2 dimensions, glm::vec3 clearColor = glm::vec3(0), bool doClear = true)
        : fb(fb)
        , clearColor(clearColor)
        , doClear(doClear)
        , scale(0)
        , dimensions(dimensions)
    { }
    /**
     * Allow subclasses to be destroyed properly by RenderPass ptrs
     */
    virtual ~RenderPass()
    { }
    std::shared_ptr<FrameBuffer> getFrameBuffer() { return fb; }
    glm::vec3 getClearColor() const{ return clearColor; }
    /**
    * Called by the scene in order of priority
    */
    virtual void executeRender() 
    {
        GLuint prevFBO = 0;
        // GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
        // but they all map to 0x8CA6
        GL_CALL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&prevFBO)));
        //Bind back buffer if shared ptr is null
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb ? fb->getName() : 0));
#if _DEBUG //Only do this check in debug
        GLint st = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (st != GL_FRAMEBUFFER_COMPLETE)
        {
            if (st == GL_FRAMEBUFFER_UNDEFINED)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_UNDEFINED", "FrameBuffer is default FrameBuffer, which doesn't exist.");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", "Atleast one attachment point is nolonger valid or has size 0.");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", "No images are attached to the framebuffer.");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
            }
            else if (st == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
            }
            else if (st == GL_FRAMEBUFFER_UNSUPPORTED)
            {
                printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_UNSUPPORTED", "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions..");
            }
            else
            {
                printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "Unknown Issue.");
            }
            GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
            return;
        }
#endif //_DEBUG
        if (doClear)
        {
            glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        executeRender();
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
    }
    /**
     * Triggered when the viewport resizes, such that attatched textures can be auto resized
     */
    virtual void resize(const int width, const int height)
    {
        //TODO
    }
    virtual void attachFBO() final
    {
        //TODO
    }
private:
    const std::shared_ptr<FrameBuffer> fb;
    const glm::vec3 clearColor;
    const bool doClear;
    const float scale;
    const glm::vec2 dimensions;
protected:
    /**
     * Called by executeRender() after framebuffer has been bound
     */
    virtual void render() = 0;
};
#endif //__RenderPass_h__