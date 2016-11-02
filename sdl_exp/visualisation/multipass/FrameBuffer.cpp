#include "FrameBuffer.h"
//Constructors
FrameBuffer::FrameBuffer(Color color, Depth depth, Stencil stencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depth, stencil, scale, glm::uvec2(1), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, Color color, Depth depth, Stencil stencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depth, stencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(Color color, DepthStencil depthstencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depthstencil, scale, glm::uvec2(1), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, Color color, DepthStencil depthstencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer({ color }, depthstencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(std::initializer_list<Color> color, Depth depth, Stencil stencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, scale, glm::uvec2(1), clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<Color> color, Depth depth, Stencil stencil, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depth, stencil, 0, dimensions, clearColor, doClear)
{ }
FrameBuffer::FrameBuffer(std::initializer_list<Color> color, DepthStencil depthstencil, float scale, bool doClear, glm::vec3 clearColor)
	: FrameBuffer(color, depthstencil, scale, glm::uvec2(1), clearColor, doClear)
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
	setDrawBuffers();
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
	setDrawBuffers();
}
FrameBuffer::~FrameBuffer()
{
	//Color
	for (auto &&it = colorConfs.begin(); it != colorConfs.end(); ++it)
	{
		auto &&it2 = colorNames.find(it->first);
		//Skip if unmanaged
		if (it->second.texName != 0)
			continue;
		if (it->second.type == Texture&&it2!=colorNames.end()){
			GL_CALL(glDeleteTextures(1, &it2->second));
		}
		else if (it->second.type == RenderBuffer&&it2 != colorNames.end())
			GL_CALL(glDeleteRenderbuffers(1, &it2->second));
		colorNames.erase(it2);
	}
	colorConfs.clear();
	//DepthStencil
	if (depthStencilConf.type == Texture&&depthStencilConf.texName==0)
	{
		GL_CALL(glDeleteTextures(1, &depthName));
		stencilName = depthName;
	}
	else if (depthStencilConf.type == RenderBuffer&&depthStencilConf.texName == 0)
	{
		GL_CALL(glDeleteRenderbuffers(1, &depthName));
		stencilName = depthName;
	}
	else
	{
		//Depth
		if (depthConf.type == Texture&&depthConf.texName == 0){
			GL_CALL(glDeleteTextures(1, &depthName));
		}
		else if (depthConf.type == RenderBuffer&&depthConf.texName == 0)
			GL_CALL(glDeleteRenderbuffers(1, &depthName));
		//Stencil
		if (stencilConf.type == Texture&&stencilConf.texName == 0){
			GL_CALL(glDeleteTextures(1, &stencilName));
		}
		else if (stencilConf.type == RenderBuffer&&stencilConf.texName == 0)
			GL_CALL(glDeleteRenderbuffers(1, &stencilName));
	}
	//FrameBuffer
	glDeleteFramebuffers(1, &name);
}
//Internal loaders
void FrameBuffer::makeColor()
{
	for (auto &&it = colorConfs.begin(); it != colorConfs.end(); ++it)
		makeColor(it->first);
}
void FrameBuffer::makeColor(GLuint attachPt)
{
	if (colorConfs[attachPt].type != Disabled)
	{
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
		if (colorConfs[attachPt].type == Texture)
		{
			if (colorConfs[attachPt].texName == 0)
			{
				//If it doesn't exist, make, 1st if potentially redundant
				if (colorNames.find(attachPt) == colorNames.end())
					colorNames[attachPt] = 0;
				if (colorNames[attachPt] == 0)
					GL_CALL(glGenTextures(1, &colorNames[attachPt]));

				GL_CALL(glBindTexture(GL_TEXTURE_2D, colorNames[attachPt]));

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
			GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachPt, GL_TEXTURE_2D, colorNames[attachPt], 0));
		}
		else if (colorConfs[attachPt].type == RenderBuffer)
		{
			if (colorConfs[attachPt].texName == 0)
			{
				//If it doesn't exist, make, 1st if potentially redundant
				if (colorNames.find(attachPt) == colorNames.end())
					colorNames[attachPt] = 0;
				if (colorNames[attachPt] == 0)
					GL_CALL(glGenRenderbuffers(1, &colorNames[attachPt]));
				//Set storage
				GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, colorNames[attachPt]));
				GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, colorConfs[attachPt].colorInternalFormat, dimensions.x, dimensions.y));
				GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
			}
			else
				colorNames[attachPt] = colorConfs[attachPt].texName;
			//Bind to our framebuffer
			GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachPt, GL_RENDERBUFFER, colorNames[attachPt]));
		}
		//drawBuffs.insert(GL_COLOR_ATTACHMENT0 + attachPt);
		//GL_CALL(glDrawBuffers(drawBuffs.size(), std::vector<GLenum>(drawBuffs.begin(), drawBuffs.end()).data()));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeDepthStencil()
{
	if (depthStencilConf.type != Disabled)
	{
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
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
				GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, depthStencilConf.colorInternalFormat, dimensions.x, dimensions.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr));
				printf("####Warning depth stencil internal format hardcoded.\n");
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
			GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthName, 0));
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
			GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthName));
		}
		//drawBuffs.insert(GL_DEPTH_STENCIL_ATTACHMENT);
		//GL_CALL(glDrawBuffers(drawBuffs.size(), std::vector<GLenum>(drawBuffs.begin(), drawBuffs.end()).data()));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeDepth()
{
	if (depthConf.type != Disabled)
	{
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
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
			GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthName, 0));
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
			GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthName));
		}
		//drawBuffs.insert(GL_DEPTH_ATTACHMENT);
		//GL_CALL(glDrawBuffers(drawBuffs.size(), std::vector<GLenum>(drawBuffs.begin(), drawBuffs.end()).data()));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::makeStencil()
{
	if (stencilConf.type != Disabled)
	{
		GLuint prevFBO = getActiveFB();
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
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
			GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilName, 0));
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
			//Bind the renderbuffer to our framebuffer
			GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilName));
		}
		//drawBuffs.insert(GL_STENCIL_ATTACHMENT);
		//GL_CALL(glDrawBuffers(drawBuffs.size(), std::vector<GLenum>(drawBuffs.begin(), drawBuffs.end()).data()));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
	}
}
void FrameBuffer::setDrawBuffers()
{
	GLuint prevFBO = getActiveFB();
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
	//Allocate array
	unsigned int itemCt = colorNames.size()>0 ? colorNames.size() : 1;
	GLenum *drawBuffsArr = (GLenum *)malloc(sizeof(GLenum)*itemCt);
	//Fill Array
	if (colorNames.size()>0)
	{
		unsigned int i = 0;
		for (auto && it: colorNames)
		{
			drawBuffsArr[i] = GL_COLOR_ATTACHMENT0 + it.first;
			i++;
		}
	}
	else
		drawBuffsArr[0] = GL_NONE;
	//Pass to GL
	GL_CALL(glDrawBuffers(itemCt, drawBuffsArr));
	//Free Array
	free(drawBuffsArr);
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
}
//Functional methods
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
		if (width>0 && height>0)
		{
			dimensions = glm::ivec2(ceil(width*scale), ceil(height*scale));
			makeColor();
			makeDepth();
			makeStencil();
			makeDepthStencil();
		}
	}
}
bool FrameBuffer::use() 
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
//Name Getters
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
