#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(Color color, Depth depth, Stencil stencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depth, stencil, scale, glm::uvec2(0), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, Color color, Depth depth, Stencil stencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depth, stencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(Color color, DepthStencil depthstencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depthstencil, scale, glm::uvec2(0), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, Color color, DepthStencil depthstencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depthstencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(std::initializer_list<Color> color, Depth depth, Stencil stencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, scale, glm::uvec2(0), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<Color> color, Depth depth, Stencil stencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(std::initializer_list<Color> color, DepthStencil depthstencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depthstencil, scale, glm::uvec2(0), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<Color> color, DepthStencil depthstencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depthstencil, 0, dimensions, clearColor, doClear)
{ }

FrameBuffer::FrameBuffer(std::initializer_list<Color> color, Depth depth, Stencil stencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear)
	: scale(scale)
	, dimensions(dimensions)
	, clearColor(clearColor)
	, doClear(doClear)
	, depthStencilConf({ Disabled })
	, depthConf(depth)
	, depthName(0)
	, stencilConf(stencil)
	, stencilName(0)
{
	GL_CALL(glGenFramebuffers(1, &name));
	for (Color c : color)
		addColorAttachment(c);
	makeDepth();
	makeStencil();
}
FrameBuffer::FrameBuffer(std::initializer_list<Color> color, DepthStencil depthstencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear)
	: scale(scale)
	, dimensions(dimensions)
	, clearColor(clearColor)
	, doClear(doClear)
	, depthStencilConf(depthstencil)
	, depthConf({ Disabled })
	, depthName(0)
	, stencilConf({ Disabled })
	, stencilName(0)
{
	GL_CALL(glGenFramebuffers(1, &name));
	for (Color c : color)
		addColorAttachment(c);
	makeDepthStencil();
}
FrameBuffer::~FrameBuffer()
{
	//Color
	for (auto &&it = colorConfs.begin(); it != colorConfs.end(); ++it)
	{
		auto &&it2 = colorNames.find(it->first);
		if (it->second.type == Texture&&it2!=colorNames.end()){
			GL_CALL(glDeleteTextures(1, &it2->second));
		}
		else if (it->second.type == RenderBuffer&&it2 != colorNames.end())
			GL_CALL(glDeleteRenderbuffers(1, &it2->second));
		colorNames.erase(it2);
	}
	colorConfs.clear();
	//DepthStencil
	if (depthStencilConf.type == Texture)
	{
		GL_CALL(glDeleteTextures(1, &depthName));
		stencilName = depthName;
	}
	else if (depthStencilConf.type == RenderBuffer)
	{
		GL_CALL(glDeleteRenderbuffers(1, &depthName));
		stencilName = depthName;
	}
	else
	{
		//Depth
		if (depthConf.type == Texture){
			GL_CALL(glDeleteTextures(1, &depthName));
		}
		else if (depthConf.type == RenderBuffer)
			GL_CALL(glDeleteRenderbuffers(1, &depthName));
		//Stencil
		if (stencilConf.type == Texture){
			GL_CALL(glDeleteTextures(1, &stencilName));
		}else if (stencilConf.type == RenderBuffer)
			GL_CALL(glDeleteRenderbuffers(1, &stencilName));
	}
	//FrameBuffer
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
			auto&& it = colorNames.find(attachPt);
			if (it==colorNames.end() || it->second == 0)
				GL_CALL(glGenTextures(1, &it->second));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, it->second));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, colorConfs[attachPt].colorInternalFormat, dimensions.x, dimensions.y, 0, colorConfs[attachPt].colorFormat, colorConfs[attachPt].colorType, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
			colorNames[attachPt] = colorConfs[attachPt].texName;
		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachPt, GL_TEXTURE_2D, colorNames[attachPt], 0));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (colorConfs[attachPt].type == RenderBuffer)
	{
		if (colorConfs[attachPt].texName == 0)
		{
			//If it doesn't exist, make
			auto&& it = colorNames.find(attachPt);
			if (it == colorNames.end() || it->second == 0)
				GL_CALL(glGenRenderbuffers(1, &it->second));
			//Set storage
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, it->second));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, colorConfs[attachPt].colorInternalFormat, dimensions.x, dimensions.y));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}
		else
			colorNames[attachPt] = colorConfs[attachPt].texName;
		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachPt, GL_RENDERBUFFER, colorNames[attachPt]));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeDepthStencil()
{
	if (depthStencilConf.type == Texture)
	{
		if (depthStencilConf.texName == 0)
		{
			//If it doesn't exist
			if (depthName == 0 && stencilName==0)
			{
				GL_CALL(glGenTextures(1, &depthName));
				stencilName = depthName;
			}

			GL_CALL(glBindTexture(GL_TEXTURE_2D, depthName));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, depthStencilConf.colorInternalFormat, dimensions.x, dimensions.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
		{
			depthName = depthStencilConf.texName;
			stencilName = depthName;
		}

		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthName, 0));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (depthStencilConf.type == RenderBuffer)
	{
		if (depthStencilConf.texName == 0)
		{
			//If it doesn't exist, make
			if (depthName == 0 && stencilName == 0)
			{
				GL_CALL(glGenRenderbuffers(1, &depthName));
				stencilName = depthName;
			}
			//Set storage
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthName));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, depthStencilConf.colorInternalFormat, dimensions.x, dimensions.y));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}
		else
		{
			depthName = depthStencilConf.texName;
			stencilName = depthName;
		}

		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthName));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeDepth()
{
	if (depthConf.type == Texture)
	{
		if (depthConf.texName == 0)
		{
			//If it doesn't exist
			if (depthName == 0)
				GL_CALL(glGenTextures(1, &depthName));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, depthName));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, depthConf.colorInternalFormat, dimensions.x, dimensions.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
			depthName = depthConf.texName;
		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthName, 0));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (depthConf.type == RenderBuffer)
	{
		if (depthConf.texName == 0)
		{
			//If it doesn't exist, make
			if (depthName == 0)
				GL_CALL(glGenRenderbuffers(1, &depthName));
			//Set storage
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthName));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, depthConf.colorInternalFormat, dimensions.x, dimensions.y));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}
		else
			depthName = depthConf.texName;

		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthName));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeStencil()
{
	if (stencilConf.type == Texture)
	{
		if (stencilConf.texName == 0)
		{
			//If it doesn't exist
			if (stencilName == 0)
				GL_CALL(glGenTextures(1, &stencilName));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, stencilName));

			//Size the texture
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, dimensions.x, dimensions.y, 0, GL_STENCIL_INDEX8, GL_UNSIGNED_BYTE, nullptr));

			//Config for mipmap access
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
			stencilName = stencilConf.texName;
		//Bind the tex to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilName, 0));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
	else if (stencilConf.type == RenderBuffer)
	{
		if (stencilConf.texName == 0)
		{
			//If it doesn't exist, make
			if (stencilName == 0)
				GL_CALL(glGenRenderbuffers(1, &stencilName));
			//Set storage
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, stencilName));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, dimensions.x, dimensions.y));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}
		else
			stencilName = stencilConf.texName;

		//Bind to our framebuffer
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilName));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
GLuint FrameBuffer::getActiveFB()
{
	GLuint prevFB = 0;
	// GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
	// but they all map to 0x8CA6
	GL_CALL(glGetIntegerv(0x8CA6, reinterpret_cast<GLint*>(&prevFB)));
	return prevFB;
}
bool FrameBuffer::isValid() const
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
		makeDepthStencil();
	}
}
bool FrameBuffer::use() const
{
#if _DEBUG //Only do this check in debug
	GLuint prevFBO = getActiveFB();
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
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
#else
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
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


GLuint FrameBuffer::getColorTextureName(GLuint attachPt) const
{
	auto && it = colorConfs.find(attachPt);
	if (it != colorConfs.end()&&it->second.type==Texture)
	{
		auto && it2 = colorNames.find(attachPt);
		if (it2 != colorNames.end())
			return it2->second;
	}
	return 0;
}
GLuint FrameBuffer::getDepthTextureName() const
{
	if (depthConf.type == Texture || depthStencilConf.type == Texture)
		return depthName;
	return 0;
}
GLuint FrameBuffer::getStencilTextureName() const
{
	if (stencilConf.type == Texture || depthStencilConf.type == Texture)
		return stencilName;
	return 0;
}
GLuint FrameBuffer::getDepthStencilTextureName() const
{
	if (depthStencilConf.type == Texture)
		return depthName;
	return 0;
}
GLuint FrameBuffer::getColorRenderBufferName(GLuint attachPt) const
{
	auto && it = colorConfs.find(attachPt);
	if (it != colorConfs.end() && it->second.type == RenderBuffer)
	{
		auto && it2 = colorNames.find(attachPt);
		if (it2 != colorNames.end())
			return it2->second;
	}
	return 0;
}
GLuint FrameBuffer::getDepthRenderBufferName() const
{
	if (depthConf.type == RenderBuffer || depthStencilConf.type == RenderBuffer)
		return depthName;
	return 0;
}
GLuint FrameBuffer::getStencilRenderBufferName() const
{
	if (stencilConf.type == RenderBuffer || depthStencilConf.type == RenderBuffer)
		return stencilName;
	return 0;
}
GLuint FrameBuffer::getDepthStencilRenderBufferName() const
{
	if (depthStencilConf.type == RenderBuffer)
		return depthName;
	return 0;
}
