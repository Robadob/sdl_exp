#ifndef __CubeMapFrameBuffer_h__
#define __CubeMapFrameBuffer_h__
#include "FrameBuffer.h"
#include "../texture/TextureCubeMap.h"

class CubeMapFrameBuffer
{
	GLuint name[6];
	GLuint renderBuf;
	std::shared_ptr<TextureCubeMap> colorTex;
	/**
	 * The clear colour to clear the frame with prior to rendering
	 */
	glm::vec4 clearColor;
	/**
	 * Whether to clear the frame prior to rendering
	 */
	bool doClearColor, doClearDepth;
	unsigned int widthHeight;
public:    
	static const glm::vec3 VIEW_DIRECTION[6];
	static const glm::vec3 VIEW_UP[6];
	enum Face
	{
		Left = 0,
		Right = 1,
		Up = 2,
		Down = 3,
		Front = 4,
		Back = 5
	};
	CubeMapFrameBuffer(unsigned int widthHeight = 2048, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	~CubeMapFrameBuffer();
	/**
	* Binds the framebuffer
	* @return True if the framebuffer is 'complete' and was bound
	*/

	/**
	* Cube map frame buffer has 6 seperate framebuffers, so it's not really intended to be used like a regular framebuffer
	*/
	bool use(Face f);
	glm::mat4 getViewMat(Face f, glm::vec3 location) const;
	glm::mat4 getProjecitonMat() const;
	/**
	* @param widthHeight The new viewport width and height (must be equal)
	* Resizes the internal images according to the specifed dimensions
	*/
	void resize(unsigned int widthHeight);
	void setDoClearDepth(const bool &d) { doClearDepth = d; }
	void setDoClearColor(const bool &c) { doClearColor = c; }
};

#endif //__CubeMapFrameBuffer_h__