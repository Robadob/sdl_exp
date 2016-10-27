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
	, depthConf(depth)
	, stencilConf(stencil)
	, colorName(0)
{
	GL_CALL(glGenFramebuffers(1, &name));
	addColorAttachment(color);
	makeDepth();
	makeStencil();
}
FrameBuffer::~FrameBuffer()
{
	for (auto &&it = colorConfs.begin(); it != colorConfs.end(); ++it)
	{
		if (it->second.type == Texture)
			GL_CALL(glDeleteTextures(1, &colorName));
		if (it->second.type == RenderBuffer)
			GL_CALL(glDeleteRenderbuffers(1, &colorName));
	}
	colorConfs.clear();
	glDeleteFramebuffers(1, &name);
}
void FrameBuffer::makeColor()
{
	for (auto &&it = colorConfs.begin(); it != colorConfs.end(); ++it)
		makeColor(it->first);
}
void FrameBuffer::makeColor(GLuint attachPt)
{
	if (colorConfs[attachPt].type == Texture)
	{
		if (colorConfs[attachPt].texName == 0)
		{
			//If it doesn't exist
			if (colorName == 0)
				GL_CALL(glGenTextures(1, &colorName));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, colorName));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, colorConfs[attachPt].colorInternalFormat, dimensions.x, dimensions.y, 0, colorConfs[attachPt].colorFormat, colorConfs[attachPt].colorType, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
			colorName = colorConfs[attachPt].texName;
		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));

		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorConfs[attachPt].attachmentId, GL_TEXTURE_2D, colorName, 0));

		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (colorConfs[attachPt].type == RenderBuffer)
	{
		//If it doesn't exist, make
		if (colorName == 0)
			GL_CALL(glGenRenderbuffers(1, &colorName));
		//Set storage
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, colorName));
		GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, colorConfs[attachPt].colorInternalFormat, dimensions.x, dimensions.y));
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachPt, GL_RENDERBUFFER, colorName));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}

}
void FrameBuffer::makeDepth()
{
	
}
void FrameBuffer::makeStencil()
{
	
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
	if (scale > 0)
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
		//Rebind prev buffer on failure
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
		return false;
	}
#endif //_DEBUG

	if (doClear)
	{
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, 1));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
	GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
	return true;
}

int FrameBuffer::addColorAttachment(FrameBuffer::Color attachment)
{	
	//Find the first free key	
	GLint attachPt = 0;
	GLint maxColorAttach = getMaxColorAttachments();
	for (attachPt; attachPt < maxColorAttach; ++attachPt)
	{
		//If key doesn't exist, break
		if (colorConfs.count(attachPt) == 0)
			break;
	}
	if (attachPt >= maxColorAttach)
	{
		return -1;
	}
	colorConfs.emplace(attachPt, attachment);
	makeColor(attachPt);
	return attachPt;
}
int FrameBuffer::getMaxColorAttachments()
{
	int rtn = 0;
	GL_CALL(glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &rtn));
	return rtn;
}