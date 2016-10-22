#ifndef __FrameBuffer_h__
#define __FrameBuffer_h__
#include "../GLcheck.h"
/**
 * https://open.gl/framebuffers
 * Need to tie this to resize events somehow
 */
class FrameBuffer
{
public:
    FrameBuffer()
    {
        glGenFramebuffers(1, &name);
    }
    ~FrameBuffer()
    {
        glDeleteFramebuffers(1, &name);
    }
    bool isValid()
    {
        GLuint prevFBO = 0;
        // GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
        // but they all map to 0x8CA6
        GL_CALL(glGetIntegerv(0x8CA6, (GLint*)&prevFBO));

        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
        GLint st = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
        return st == GL_FRAMEBUFFER_COMPLETE;        
    }
    GLuint getName() const{ return name; }
    /**
     * Todo, probably 1 per attatchment may work better?
     */
    void addAttachment()
    {
        
    }
private:

    GLuint name;
};

#endif