#ifndef __RenderBuffer_h__
#define __RenderBuffer_h__

#include "../util/GLcheck.h"
#include <memory>
#include <glm/vec2.hpp>
#include "../interface/RenderTarget.h"

/**
 * Represents an OpenGL renderbuffer
 * These can be rendered to and blit, but not sampled like texture's
 */
class RenderBuffer : public RenderTarget
{
public:
	/**
	 * Returns a new renderbuffer of the given dimensions and format
	 * @param dimensions Size of the renderbuffer
	 * @param internalFormat Internal format type of the renderbuffer
	 * @param samples The number of samples, if 0 is passed multisampling will be disabled
	 */
	static std::shared_ptr<RenderBuffer> make(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples = 0);
	~RenderBuffer();
	/**
	 * Resizes the renderbuffer
	 * @param dimensions The new size of the renderbuffer
	 */
	void resize(const glm::uvec2 dimensions) override;
	/**
	 * Returns the OpenGL name as allocated by glGenRenderbuffers()
	 */
	GLuint getName() const override { return name; }
private:
	/**
	 * private constructor
	 * @see make()
	 */
	RenderBuffer(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples);
	/**
	* Calls glGenRenderbuffers() returning the generated texture name
	*/
	static GLuint genName();
	const GLuint name;
	const unsigned int samples;
	const GLenum internalFormat;
	glm::uvec2 dimensions;
};

#endif //__RenderBuffer_h__