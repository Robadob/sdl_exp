#include "BackBuffer.h"
BackBuffer::BackBuffer(bool doClear, glm::vec3 clearColor)
	: dimensions(800, 600)//Something simple and valid, resize() should really be called directly after construction
	, doClear(doClear)
	, clearColor(clearColor)
{ }
void BackBuffer::resize(const glm::uvec2 &dims)
{
	if (dims.x>0 && dims.y>0)
		this->dimensions = dims;
}
bool BackBuffer::use()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
	if (doClear)
	{
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, 1));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
	GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
	//The backbuffer cannot be modified, therefore it must always be complete.
	return true;
}