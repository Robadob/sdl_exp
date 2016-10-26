#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(Color color, Depth depth, Stencil stencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, scale, glm::uvec2(0), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(Color color, Depth depth, Stencil stencil, glm::uvec2 dimensions, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(Color color, Depth depth, Stencil stencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear)
	: scale(scale)
	, dimensions(dimensions)
	, clearColor(clearColor)
	, doClear(doClear)
	, colorConf(color)
	, depthConf(depth)
	, stencilConf(stencil)
	, colorName(0)
{
	GL_CALL(glGenFramebuffers(1, &name));
	makeColor();
	makeDepth();
	makeStencil();
}
FrameBuffer::~FrameBuffer()
{
	if (colorConf.type == Texture)
		GL_CALL(glDeleteTextures(1, &colorName));
	if (colorConf.type == RenderBuffer)
		GL_CALL(glDeleteRenderbuffers(1, &colorName));
	glDeleteFramebuffers(1, &name);
}
void FrameBuffer::makeColor()
{
	if (colorConf.type == Texture)
	{
		if (colorConf.texName == 0)
		{
			//If it doesn't exist
			if (colorName == 0)
				GL_CALL(glGenTextures(1, &colorName));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, colorName));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, colorConf.colorInternalFormat, dimensions.x, dimensions.y, 0, colorConf.colorFormat, colorConf.colorType, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
			colorName = colorConf.texName;
		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));

		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorConf.attachmentId, GL_TEXTURE_2D, colorName, 0));

		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (colorConf.type == RenderBuffer)
	{
		//If it doesn't exist, make
		if (colorName == 0)
			GL_CALL(glGenRenderbuffers(1, &colorName));
		//Set storage
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, colorName));
		GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, colorConf.colorInternalFormat, dimensions.x, dimensions.y));
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorConf.attachmentId, GL_RENDERBUFFER, colorName));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}

}
GLuint FrameBuffer::getActiveFB()
{
	GLuint prevFB = 0;
	// GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
	// but they all map to 0x8CA6
	GL_CALL(glGetIntegerv(0x8CA6, (GLint*)&prevFB));
	return prevFB;
}
bool FrameBuffer::isValid()
{
	GLuint prevFBO = getActiveFB();
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
		makeColor();
		makeDepth();
		makeStencil();
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
			GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, 1));
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		}
		GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
		return true;
	}
	//Rebind prev buffer on failure
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	return false;
}

void FrameBuffer::setColorBuffer()
{
	
}