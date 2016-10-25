#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
	glGenFramebuffers(1, &name);
}
FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &name);
}

bool FrameBuffer::isValid()
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
void FrameBuffer::resize(int width, int height)
{
	if (scale>0)
	{
		dimensions = glm::ivec2(ceil(width*scale), ceil(height*scale));
		//todo: resize textures
	}
}
bool FrameBuffer::use()
{
	GLuint prevFBO = 0;
	// GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
	// but they all map to 0x8CA6
	GL_CALL(glGetIntegerv(0x8CA6, (GLint*)&prevFBO));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
	GLint st = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (st == GL_FRAMEBUFFER_COMPLETE)
	{
		if (doClear)
		{
			GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		}
		GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
		return true;
	}
	//Rebind prev buffer on failure
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	return false;
}