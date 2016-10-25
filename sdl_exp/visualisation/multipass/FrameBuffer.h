#ifndef __FrameBuffer_h__
#define __FrameBuffer_h__
#include "../GLcheck.h"
#include "glm/glm.hpp"
/**
 * https://open.gl/framebuffers
 * Need to tie this to resize events somehow
 */
class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();
	bool isValid();
    GLuint getName() const{ return name; }
    /**
     * Todo, probably 1 per attatchment may work better?
     */
    void addAttachment()
    {
        
    }
	void resize(int width, int height);
	bool use();
private:
	float scale;
	glm::ivec2 dimensions;
    GLuint name;
	glm::vec4 clearColor;
	bool doClear;
};

#endif