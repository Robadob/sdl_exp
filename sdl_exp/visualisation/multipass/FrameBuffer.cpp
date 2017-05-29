#include "FrameBuffer.h"
//Constructors

typedef FrameBufferAttachment FBA;
//Scaling FrameBuffers
FrameBuffer::FrameBuffer(FBA color, FBA depth, FBA stencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, depth, stencil, FBAFactory::Disabled(), scale, glm::uvec2(1), samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(FBA color, FBA depthstencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer({ color }, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, scale, glm::uvec2(1), samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depth, FBA stencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer(color, depth, stencil, FBAFactory::Disabled(), scale, glm::uvec2(1), samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depthstencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer(color, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, scale, glm::uvec2(1), samples, clearColor, doClear)
{}
//Fixed FrameBuffers
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, FBA color, FBA depth, FBA stencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, depth, stencil, FBAFactory::Disabled(), 0, dimensions, samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, FBA color, FBA depthstencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, 0, dimensions, samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FBA> color, FBA depth, FBA stencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer(color, depth, stencil, FBAFactory::Disabled(), 0, dimensions, samples, clearColor, doClear)
{}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FBA> color, FBA depthstencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer(color, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, 0, dimensions, samples, clearColor, doClear)
{}
//Actual constructor
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depth, FBA stencil, FBA depthstencil, float scale, glm::uvec2 dimensions, unsigned int samples, glm::vec3 clearColor, bool doClear)
	: scale(scale)
    , dimensions(dimensions)
    , samples(samples)
	, clearColor(glm::vec4(clearColor,1.0f))
    , doClear(doClear)
    , depthStencil(depthstencil)
	, depth(depth)
	, stencil(stencil)
{
#ifdef _DEBUG
    int r;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &r);
    assert(static_cast<unsigned int>(r) >= dimensions.x);
    assert(static_cast<unsigned int>(r) >= dimensions.y);
#endif
    assert(depthstencil.Type() == FBA::Disabled || (depth.Type() == FBA::Disabled &&stencil.Type() == FBA::Disabled));
    assert(color.size() <= static_cast<unsigned int>(getMaxColorAttachments()));
    GL_CALL(glGenFramebuffers(1, &name));    
    {//Assign an attachment point to each color attachment
        unsigned int i = 0;
        for (FBA c : color)
        {
            if (c.Class() != FBA::Disabled)
            {
                colors.emplace(i++, c);
            }
        }
    }
    makeAttachments();
	setDrawBuffers();
}
FrameBuffer::~FrameBuffer()
{
	//Color
	for (auto &&it = colors.begin(); it != colors.end(); ++it)
	{
		//Skip if unmanaged
		if (it->second.conf.TexName() != 0)
			continue;
        if (it->second.conf.Type() == FBA::Texture){
            GL_CALL(glDeleteTextures(1, &it->second.texName));
		}
        else if (it->second.conf.Type() == FBA::RenderBuffer){
            GL_CALL(glDeleteRenderbuffers(1, &it->second.texName));
        }
	}
	colors.clear();
	//DepthStencil
    if (depthStencil.conf.Type() == FBA::Texture&&depthStencil.conf.TexName() == 0)
	{
        GL_CALL(glDeleteTextures(1, &depthStencil.texName));
	}
    else if (depthStencil.conf.Type() == FBA::RenderBuffer&&depthStencil.conf.TexName() == 0)
	{
        GL_CALL(glDeleteRenderbuffers(1, &depthStencil.texName));
	}
    else
    {
        //Depth
        if (depth.conf.Type() == FBA::Texture&&depth.conf.TexName() == 0){
            GL_CALL(glDeleteTextures(1, &depth.texName));
        }
        else if (depth.conf.Type() == FBA::RenderBuffer&&depth.conf.TexName() == 0) {
            GL_CALL(glDeleteRenderbuffers(1, &depth.texName));
        }
        //Stencil
        if (stencil.conf.Type() == FBA::Texture&&stencil.conf.TexName() == 0){
            GL_CALL(glDeleteTextures(1, &stencil.texName));
        }
        else if (stencil.conf.Type() == FBA::RenderBuffer&&stencil.conf.TexName() == 0){
            GL_CALL(glDeleteRenderbuffers(1, &stencil.texName));
        }
	}
	//FrameBuffer
    GL_CALL(glDeleteFramebuffers(1, &name));
}
//Internal loaders
void FrameBuffer::makeAttachments()
{
	for (auto &&it = colors.begin(); it != colors.end(); ++it)
    {
        assert(it->second.conf.Class() == FBA::Color);
        makeAttachment(it->second.conf, GL_COLOR_ATTACHMENT0 + it->first, &it->second.texName);
    }
    if (depthStencil.conf.Type() != FBA::Disabled)
    {
        assert(depthStencil.conf.Class() == FBA::DepthStencil);
        makeAttachment(depthStencil.conf, GL_DEPTH_STENCIL_ATTACHMENT, &depthStencil.texName);
    }
    else
    {
        if (depth.conf.Type() != FBA::Disabled)
        {
            assert(depth.conf.Class() == FBA::Depth);
            makeAttachment(depth.conf, GL_DEPTH_ATTACHMENT, &depth.texName);
        }
        if (stencil.conf.Type() != FBA::Disabled)
        {
            assert(stencil.conf.Class() == FBA::Stencil);
            makeAttachment(stencil.conf, GL_STENCIL_ATTACHMENT, &stencil.texName);
        }
    }
}
void FrameBuffer::makeAttachment(const FBA &attachmentConfig, GLenum attachPoint, GLuint *texNameOut) const
{
    if (attachmentConfig.Type() != FBA::Disabled)
    {
        GLuint prevFBO = getActiveFB();
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
        if (attachmentConfig.Type() == FBA::Texture)
        {
            if (attachmentConfig.TexName() == 0)
            {
                if (*texNameOut==0)//Don't generate a new texture if resizing
                    GL_CALL(glGenTextures(1, texNameOut));

                GL_CALL(glBindTexture(GL_TEXTURE_TYPE(), *texNameOut));

                //Size the texture
                if (samples)
                {//Multisampling
                    GL_CALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, attachmentConfig.InternalFormat(), dimensions.x, dimensions.y, true));
                }
                else
                {//No Multisampling
                    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, attachmentConfig.InternalFormat(), dimensions.x, dimensions.y, 0, attachmentConfig.PixelFormat(), attachmentConfig.StorageType(), nullptr));
                    //Config for mipmap access
                    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                    //Disable depth comparison by default
                    //if (attachPoint == GL_DEPTH_ATTACHMENT || attachPoint == GL_DEPTH_STENCIL_ATTACHMENT)
                    //    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE));
                }
                GL_CALL(glBindTexture(GL_TEXTURE_TYPE(), 0));
            }
            else
                *texNameOut = attachmentConfig.TexName();
            //Bind the tex to our framebuffer
            GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachPoint, GL_TEXTURE_TYPE(), *texNameOut, 0));
        }
        else if (attachmentConfig.Type() == FBA::RenderBuffer)
        {
            if (attachmentConfig.TexName() == 0)
            {
                if (*texNameOut == 0)//Don't generate a new renderbuffer if resizing
                    GL_CALL(glGenRenderbuffers(1, texNameOut));
                //Set storage
                GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, *texNameOut));
                if (samples)
                {
                    GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, attachmentConfig.InternalFormat(), dimensions.x, dimensions.y));
                }
                else
                {
                    GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, attachmentConfig.InternalFormat(), dimensions.x, dimensions.y));
                }
                GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
            }
            else
                *texNameOut = attachmentConfig.TexName();
            //Bind to our framebuffer
            GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER, *texNameOut));
        }
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
    }
}
void FrameBuffer::setDrawBuffers()
{
    GLuint prevFBO = getActiveFB();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
    //Allocate array
    unsigned int itemCt = (unsigned int)(colors.size()>0 ? colors.size() : 1);
    GLenum *drawBuffsArr = (GLenum *)malloc(sizeof(GLenum)*itemCt);
    //Fill Array
    if (colors.size()>0)
    {
        unsigned int i = 0;
        for (auto && it : colors)
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
            makeAttachments();
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
        GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
	GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
	return true;
}
//Name Getters
GLuint FrameBuffer::getColorTextureName(GLuint attachPt) const
{
	auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::Texture)
			return it->second.texName;
	return 0;
}
GLuint FrameBuffer::getDepthTextureName() const
{
    if (depth.conf.Type() == FBA::Texture)
		return depth.texName;
    if (depthStencil.conf.Type() == FBA::Texture)
        return depthStencil.texName;
	return 0;
}
GLuint FrameBuffer::getStencilTextureName() const
{
    if (stencil.conf.Type() == FBA::Texture)
        return stencil.texName;
    if (depthStencil.conf.Type() == FBA::Texture)
        return depthStencil.texName;
	return 0;
}
GLuint FrameBuffer::getDepthStencilTextureName() const
{
    if (depthStencil.conf.Type() == FBA::Texture)
        return depthStencil.texName;
	return 0;
}
GLuint FrameBuffer::getColorRenderBufferName(GLuint attachPt) const
{
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::RenderBuffer)
        return it->second.texName;
    return 0;
}
GLuint FrameBuffer::getDepthRenderBufferName() const
{
    if (depth.conf.Type() == FBA::RenderBuffer)
        return depth.texName;
    if (depthStencil.conf.Type() == FBA::RenderBuffer)
        return depthStencil.texName;
    return 0;
}
GLuint FrameBuffer::getStencilRenderBufferName() const
{
    if (stencil.conf.Type() == FBA::RenderBuffer)
        return stencil.texName;
    if (depthStencil.conf.Type() == FBA::RenderBuffer)
        return depthStencil.texName;
    return 0;
}
GLuint FrameBuffer::getDepthStencilRenderBufferName() const
{
    if (depthStencil.conf.Type() == FBA::RenderBuffer)
        return depthStencil.texName;
    return 0;
}
void FrameBuffer::disableFiltering(GLuint attachPt)
{
	auto && it = colors.find(attachPt);
	if (it != colors.end() && it->second.conf.Type() == FBA::Texture)
	{
		GL_CALL(glBindTexture(GL_TEXTURE_TYPE(), it->second.texName));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		GL_CALL(glBindTexture(GL_TEXTURE_TYPE(), 0));
	}
}