#ifndef __BackBuffer_h__
#define __BackBuffer_h__
#include "../interface/FBuffer.h"
#include "glm/glm.hpp"
/**
 * Represents the default FrameBuffer, often called the BackBuffer in double-buffered rendering
 * FrontBuffer refers to the one currently shown, which is swapped with the BackBuffer after rendering has completed
 * @note This class simply wraps the binding of Buffer 0, and clearing
 */
class BackBuffer : public FBuffer
{
public:
	/**
	 * @note Call resize() as soon after construction as you know the viewport dimensions
	 */
	BackBuffer(bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	/**
	 * @return The name of the contained GL_FRAMEBUFFER
	 * @note This it the value created by glGenFramebuffer() and used with glBindBuffer()
	 */
	GLuint getFrameBufferName() override final{ return name; };
	/**
	 * @param width The new viewport width
	 * @param height The new viewport height
	 * Resizes the internal images according to the specifed dimensions and the internal scaling factor
	 */
	void resize(const glm::uvec2 &dims) override final;
	/**
	 * Binds the framebuffer
	 * @return True if the framebuffer is 'complete' and was bound
	 */
	bool use() override final;
	const GLuint name = 0;
	glm::uvec2 dimensions;
	bool doClear;
	glm::vec3 clearColor;
};

#endif //__BackBuffer_h__