#ifndef __RenderBuffer_h__
#define __RenderBuffer_h__

#include "../util/GLcheck.h"
#include <memory>
#include <glm/vec2.hpp>
#include "../interface/RenderTarget.h"

class RenderBuffer : public RenderTarget
{
public:
	static std::shared_ptr<RenderBuffer> make(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples = 0);
	~RenderBuffer();
	void resize(const glm::uvec2 dimensions) override;
	GLuint getName() const override { return name; }
private:
	RenderBuffer(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples);
	/**
	* Calls glGenTextures() returning the generated texture name
	*/
	static GLuint genName();
	const GLuint name;
	const unsigned int samples;
	const GLenum internalFormat;
	glm::uvec2 dimensions;
};

#endif //__RenderBuffer_h__