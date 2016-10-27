#ifndef __FrameBuffer_h__
#define __FrameBuffer_h__
#include "../GLcheck.h"
#include "glm/glm.hpp"
#include <map>

/**
 * https://open.gl/framebuffers
 * Need to tie this to resize events somehow
 */
class FrameBuffer
{
public:
	enum AttatchType {Disabled, Texture, RenderBuffer};
	struct Color
	{
		/**
		 * How to provide the attatchement
		 * @note You probably want Disabled or Texture, You can't sample from a renderbuffer
		 */
		AttatchType type;
		/**
		 * The internal format of the texture
		 * @note Required for both texture and renderbuffer types
		 * @see glTexImage2D() and glRenderbufferStorage()
		 */
		GLint colorInternalFormat;//GL_RGB, GL_RGBA8
		/**
		 * The color format of the texture
		 * @note Only required for texture types
		 * @see glTexImage2D() 
		 */
		GLenum colorFormat;//GL_RGB, GL_BGRA
		/**
		 * The storage type of the texture
		 * @note Only required for texture types
		 * @see glTexImage2D() 
		 */
		GLenum colorType;//GL_UNSIGNED_BYTE
		/**
		 * Number of samples required
		 * If less than or equal to 1, multisample is disabled
		 * @note Only required for multisample renderbuffer types
		 * @see glRenderbufferStorageMultisample()
		 */
		GLsizei samples;
		/**
		 * The name of the unmanaged texture to use
		 * @note Only set this value to something other than 0 if you want to use an unmanaged texture
		 * @see This is the value which glGenTextures() creates
		 */
		GLuint texName;
	};
	struct DepthStencil
	{
		/**
		* How to provide the attatchement
		* @note You probably want Disabled or Texture, You can't sample from a renderbuffer
		*/
		AttatchType type;
		/**
		* The internal format of the texture
		* @note Required for both texture and renderbuffer types
		* @see glTexImage2D() and glRenderbufferStorage()
		*/
		GLint colorInternalFormat;//GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8
		/**
		* The color format of the texture
		* @note Only required for texture types
		* @note No variation permitted for depth textures
		* @see glTexImage2D()
		*/
		//GL_DEPTH_STENCIL (for the two depth_stencil internal formats)
		/**
		* The storage type of the texture
		* @note Only required for texture types
		* @see glTexImage2D()
		*/
		//const GLenum colorType = GL_UNSIGNED_BYTE;
		/**
		* Number of samples required
		* If less than or equal to 1, multisample is disabled
		* @note Only required for multisample renderbuffer types
		* @see glRenderbufferStorageMultisample()
		*/
		GLsizei samples;
		/**
		* The name of the unmanaged texture to use
		* @note Only set this value to something other than 0 if you want to use an unmanaged texture
		* @see This is the value which glGenTextures() creates
		*/
		GLuint texName;
	};
	struct Depth
	{
		/**
		* How to provide the attatchement
		* @note You probably want Disabled or Texture, You can't sample from a renderbuffer
		*/
		AttatchType type;
		/**
		* The internal format of the texture
		* @note Required for both texture and renderbuffer types
		* @see glTexImage2D() and glRenderbufferStorage()
		*/
		GLint colorInternalFormat;//GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F
		/**
		* The color format of the texture
		* @note Only required for texture types
		* @note No variation permitted for depth textures
		* @see glTexImage2D()
		*/
		//const GLenum colorFormat = GL_DEPTH_COMPONENT;
		/**
		* The storage type of the texture
		* @note Only required for texture types
		* @see glTexImage2D()
		*/
		//const GLenum colorType = GL_UNSIGNED_BYTE;
		/**
		* Number of samples required
		* If less than or equal to 1, multisample is disabled
		* @note Only required for multisample renderbuffer types
		* @see glRenderbufferStorageMultisample()
		*/
		GLsizei samples;
		/**
		* The name of the unmanaged texture to use
		* @note Only set this value to something other than 0 if you want to use an unmanaged texture
		* @see This is the value which glGenTextures() creates
		*/
		GLuint texName;
	};
	struct Stencil
	{
		/**
		* How to provide the attatchement
		* @note You probably want Disabled or Texture, You can't sample from a renderbuffer
		*/
		AttatchType type;
		/**
		* The internal format of the texture
		* @note Required for both texture and renderbuffer types
		* @see glTexImage2D() and glRenderbufferStorage()
		*/
		GLint colorInternalFormat;//GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8, GL_STENCIL_INDEX8
		/**
		* The color format of the texture
		* @note Only required for texture types
		* @note No variation permitted for depth textures
		* @see glTexImage2D()
		*/
		//GL_STENCIL_INDEX8 (for the GL_STENCIL_INDEX8 internal format, may not be supported prior to gl 4.3)
		/**
		* The storage type of the texture
		* @note Only required for texture types
		* @see glTexImage2D()
		*/
		//const GLenum colorType = GL_UNSIGNED_BYTE;
		/**
		* Number of samples required
		* If less than or equal to 1, multisample is disabled
		* @note Only required for multisample renderbuffer types
		* @see glRenderbufferStorageMultisample()
		*/
		GLsizei samples;
		/**
		* The name of the unmanaged texture to use
		* @note Only set this value to something other than 0 if you want to use an unmanaged texture
		* @see This is the value which glGenTextures() creates
		*/
		GLuint texName;
	};
	FrameBuffer(Color color, Depth depth, Stencil stencil, float scale=1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(Color color, Depth depth, Stencil stencil, glm::uvec2 dimensions, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	~FrameBuffer();
	bool isValid();
    GLuint getName() const{ return name; }
	/**
	 * @return The color attachment point bound to. -1 if max color attachments already reached
	 * @note GTX960 reported 8 max color attachments
	 */
	int addColorAttachment(FrameBuffer::Color attachment);
	void resize(int width, int height);
	bool use();
	static int getMaxColorAttachments();
private:
	FrameBuffer(Color color, Depth depth, Stencil stencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear);
	void makeColor();
	void makeColor(GLuint attachPt);
	void makeDepthStencil();
	void makeDepth();
	void makeStencil();
	static GLuint getActiveFB();
	
	const float scale;
	glm::uvec2 dimensions;
    GLuint name;
	glm::vec3 clearColor;
	bool doClear;
	/**
	 * Key: Color attachment point
	 * Value: Color attachment configuration
	 */
	std::map<GLuint, const Color> colorConfs;
	GLuint colorName;
	const DepthStencil depthStencilConf;
	const Depth depthConf;
	GLuint depthName;
	const Stencil stencilConf;
	GLuint stencilName;
};

#endif