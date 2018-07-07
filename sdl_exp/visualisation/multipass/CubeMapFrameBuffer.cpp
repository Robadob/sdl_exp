#include "CubeMapFrameBuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../Visualisation.h"

const glm::vec3 CubeMapFrameBuffer::VIEW_DIRECTION[6] = {
	glm::vec3(+1.0f, 0.0f, 0.0f), //left
	glm::vec3(-1.0f, 0.0f, 0.0f), //right
	glm::vec3(0.0f, +1.0f, 0.0f), //up
	glm::vec3(0.0f, -1.0f, 0.0f), //down
	glm::vec3(0.0f, 0.0f, +1.0f), //front
	glm::vec3(0.0f, 0.0f, -1.0f)  //back
};
//Negate some of these values to reflect the rendered image
const glm::vec3 CubeMapFrameBuffer::VIEW_UP[6] = {
	-glm::vec3(0.0f, 1.0f, 0.0f),  //left
	-glm::vec3(0.0f, 1.0f, 0.0f),  //right
	glm::vec3(0.0f, 0.0f, 1.0f),  //up
	glm::vec3(0.0f, 0.0f, -1.0f), //down
	-glm::vec3(0.0f, 1.0f, 0.0f),  //front
	-glm::vec3(0.0f, 1.0f, 0.0f)   //back
};
CubeMapFrameBuffer::CubeMapFrameBuffer(unsigned int widthHeight, bool doClear, glm::vec3 clearColor)
	: clearColor(clearColor, 1.0f)
	, doClearColor(doClear)
	, doClearDepth(doClear)
	, widthHeight(widthHeight)
{
	GL_CALL(glGenFramebuffers(CUBE_MAP_FACE_COUNT, name));
	GLuint prevFBO = FBuffer::getActiveFB();

	colorTex = TextureCubeMap::make(widthHeight, Texture::Format(GL_RGBA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8));

	//Allocate one depth render buf for every face
	GL_CALL(glGenRenderbuffers(1, &renderBuf));
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, renderBuf));
	GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, widthHeight, widthHeight));
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	for (unsigned int i = 0; i < CUBE_MAP_FACE_COUNT; ++i)
	{
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name[i]));
		//Temporary fixed colour tex
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, colorTex->getName(), 0));
		//Temporary fixed depth tex
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuf));
		//Set draw buffers
		GLenum t = GL_COLOR_ATTACHMENT0;
		GL_CALL(glDrawBuffers(1, &t));
	}

	//Reset framebuffer
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
}
CubeMapFrameBuffer::~CubeMapFrameBuffer()
{
	//Temporary remove colour tex
	colorTex.reset();
	//Temporary remove depth tex
	GL_CALL(glDeleteRenderbuffers(1, &renderBuf));

	GL_CALL(glDeleteFramebuffers(CUBE_MAP_FACE_COUNT, name));
}


bool CubeMapFrameBuffer::use(Face f)
{
	assert(f < CUBE_MAP_FACE_COUNT);
#if _DEBUG //Only do this check in debug
	GLuint prevFBO = FBuffer::getActiveFB();
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name[f]));
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
	GLbitfield clearConfig = 0;
	if (doClearColor)
	{
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
		clearConfig |= GL_COLOR_BUFFER_BIT;
	}
	if (doClearDepth)
		clearConfig |= GL_DEPTH_BUFFER_BIT;

	GL_CALL(glClear(clearConfig));
	GL_CALL(glViewport(0, 0, widthHeight, widthHeight));
	return true;
}

glm::mat4 CubeMapFrameBuffer::getViewMat(Face f, glm::vec3 location)
{
	return glm::lookAt(location, location+VIEW_DIRECTION[f], VIEW_UP[f]);
}
glm::mat4 CubeMapFrameBuffer::getSkyBoxViewMat(Face f)
{
	return glm::lookAt(glm::vec3(0), VIEW_DIRECTION[f], VIEW_UP[f]);
}
glm::mat4 CubeMapFrameBuffer::getProjecitonMat()
{
	return glm::perspective(glm::radians(90.0f), 1.0f, Visualisation::NEAR_CLIP, Visualisation::FAR_CLIP);
}
void CubeMapFrameBuffer::resize(unsigned int widthHeight)
{
	//Resize tex
	colorTex->resize(widthHeight);
	//Resize renderbuf
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, renderBuf));
	GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, widthHeight, widthHeight));
	GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}