#ifndef __FrameBuffer_h__
#define __FrameBuffer_h__
#include "../GLcheck.h"
#include "glm/glm.hpp"
#include <map>

/**
 * https://open.gl/framebuffers
 * @todo MultisampleFrameBuffer subclass (all attachments must be the same type (tex vs renderbuffer) + samples ct)
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
		 * The name of the unmanaged texture or renderbuffer to use
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
		//GL_DEPTH_STENCIL
		/**
		* The storage type of the texture
		* @note Only required for texture types
		* @see glTexImage2D()
		*/
		//const GLenum colorType = GL_UNSIGNED_BYTE;
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
		//GLint colorInternalFormat;//GL_STENCIL_INDEX8
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
		* The name of the unmanaged texture to use
		* @note Only set this value to something other than 0 if you want to use an unmanaged texture
		* @see This is the value which glGenTextures() creates
		*/
		GLuint texName;
	};
	FrameBuffer(Color color, Depth depth = { Disabled }, Stencil stencil = { Disabled }, float scale=1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(glm::uvec2 dimensions, Color color, Depth depth = { Disabled }, Stencil stencil = { Disabled }, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(Color color, DepthStencil depthstencil = { Disabled }, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(glm::uvec2 dimensions, Color color, DepthStencil depthstencil = { Disabled }, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(std::initializer_list<Color> color, Depth depth = { Disabled }, Stencil stencil = { Disabled }, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(glm::uvec2 dimensions, std::initializer_list<Color> color, Depth depth = { Disabled }, Stencil stencil = { Disabled }, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(std::initializer_list<Color> color, DepthStencil depthstencil = { Disabled }, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	FrameBuffer(glm::uvec2 dimensions, std::initializer_list<Color> color, DepthStencil depthstencil = { Disabled }, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));

	~FrameBuffer();
	bool isValid() const;
    GLuint getName() const{ return name; }
	/**
	 * @return The color attachment point bound to. -1 if max color attachments already reached
	 * @note Probably 8 max color attachments (960GTX and HD4600 both reported)
	 */
	int addColorAttachment(FrameBuffer::Color attachment);
	void resize(int width, int height);
	bool use() const;
	static int getMaxColorAttachments();
	/**
	 * @return The name of the contained GL_FRAMEBUFFER
	 */
	GLuint getFrameBufferName() { return name; };
	/**
	 * @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
	 * @return The name of the texture bound to the specified attachment point
	 * @note 0 is returned if not bound or bound as renderbuffer
	 */
	GLuint getColorTextureName(GLuint attachPt = 0) const;
	/**
	 * @return The name of the texture bound to the specified attachment point
	 * @note 0 is returned if not bound or bound as renderbuffer
	 * @note This will return a value if you bound a Depth texture or a DepthStencil texture
	 */
	GLuint getDepthTextureName() const;
	/**
	* @return The name of the texture bound to the specified attachment point
	* @note 0 is returned if not bound or bound as renderbuffer
	 * @note This will return a value if you bound a Stencil texture or a DepthStencil texture
	*/
	GLuint getStencilTextureName() const;
	/**
	* @return The name of the texture bound to the specified attachment point
	* @note 0 is returned if not bound or bound as renderbuffer
	* @note This will only return a value if you bound a DepthStencil texture
	*/
	GLuint getDepthStencilTextureName() const;
	/**
	* @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
	* @return The name of the renderbuffer bound to the specified attachment point
	* @note 0 is returned if not bound or bound as texture
	*/
	GLuint getColorRenderBufferName(GLuint attachPt = 0) const;
	/**
	* @return The name of the renderbuffer bound to the specified attachment point
	* @note 0 is returned if not bound or bound as texture
	* @note This will return a value if you bound a Depth renderbuffer or a DepthStencil renderbuffer
	*/
	GLuint getDepthRenderBufferName() const;
	/**
	* @return The name of the renderbuffer bound to the specified attachment point
	* @note 0 is returned if not bound or bound as texture
	* @note This will return a value if you bound a Stencil renderbuffer or a DepthStencil renderbuffer
	*/
	GLuint getStencilRenderBufferName() const;
	/**
	* @return The name of the renderbuffer bound to the specified attachment point
	* @note 0 is returned if not bound or bound as texture
	* @note This will only return a value if you bound a DepthStencil renderbuffer
	*/
	GLuint getDepthStencilRenderBufferName() const;
private:
	FrameBuffer(std::initializer_list<Color> color, Depth depth, Stencil stencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear);
	FrameBuffer(std::initializer_list<Color> color, DepthStencil depthstencil, float scale, glm::uvec2 dimensions, glm::vec3 clearColor, bool doClear);
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
	/**
	 * Holds the texture/render buffer name for each color attachment
	 * @note Unless an issue occurs, every item in this map should have a corresponding item in colorConfs
	 */
	std::map<GLuint, GLuint> colorNames;
	const DepthStencil depthStencilConf;
	const Depth depthConf;
	GLuint depthName;
	const Stencil stencilConf;
	GLuint stencilName;
};

namespace Stock
{
	namespace Attachments
	{
		//Color
		static FrameBuffer::Color COLOR_DISABLED(){ return{ FrameBuffer::Disabled }; }
		static FrameBuffer::Color COLOR_TEXTURE_RGB(){ return{ FrameBuffer::Texture, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 0 }; }
		static FrameBuffer::Color COLOR_TEXTURE_RGBA(){ return{ FrameBuffer::Texture, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 0 }; }
		static FrameBuffer::Color COLOR_TEXTURE_CUSTOM(GLuint name){ return{ FrameBuffer::Texture, 0, 0, 0, name }; }
		static FrameBuffer::Color COLOR_RENDERBUFFER_RGB(){ return{ FrameBuffer::RenderBuffer, GL_RGB, 0, 0, 0 }; }
		static FrameBuffer::Color COLOR_RENDERBUFFER_RGBA(){ return{ FrameBuffer::RenderBuffer, GL_RGBA8, 0, 0, 0 }; }
		static FrameBuffer::Color COLOR_RENDERBUFFER_CUSTOM(GLuint name){ return{ FrameBuffer::RenderBuffer, 0, 0, 0, name }; }
		//DepthStencil
		static FrameBuffer::DepthStencil DEPTH_STENCIL_DISABLED(){ return{ FrameBuffer::Disabled }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_TEXTURE(){ return{ FrameBuffer::Texture, GL_DEPTH24_STENCIL8, 0 }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_TEXTURE_32F(){ return{ FrameBuffer::Texture, GL_DEPTH32F_STENCIL8, 0 }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_TEXTURE_CUSTOM(GLuint name){ return{ FrameBuffer::Texture, 0, name }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_RENDERBUFFER(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH24_STENCIL8, 0 }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_RENDERBUFFER_32f(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH32F_STENCIL8, 0 }; }
		static FrameBuffer::DepthStencil DEPTH_STENCIL_RENDERBUFFER_CUSTOM(GLuint name){ return{ FrameBuffer::RenderBuffer, 0, name }; }
		//Depth
		static FrameBuffer::Depth DEPTH_DISABLED(){ return{ FrameBuffer::Disabled }; }
		static FrameBuffer::Depth DEPTH_TEXTURE(){ return{ FrameBuffer::Texture, GL_DEPTH_COMPONENT24, 0 }; }
		static FrameBuffer::Depth DEPTH_TEXTURE_16(){ return{ FrameBuffer::Texture, GL_DEPTH_COMPONENT16, 0 }; }
		static FrameBuffer::Depth DEPTH_TEXTURE_32(){ return{ FrameBuffer::Texture, GL_DEPTH_COMPONENT32, 0 }; }
		static FrameBuffer::Depth DEPTH_TEXTURE_32F(){ return{ FrameBuffer::Texture, GL_DEPTH_COMPONENT32F, 0 }; }
		static FrameBuffer::Depth DEPTH_TEXTURE_CUSTOM(GLuint name){ return{ FrameBuffer::Texture, 0, name }; }
		static FrameBuffer::Depth DEPTH_RENDERBUFFER(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH_COMPONENT24, 0 }; }
		static FrameBuffer::Depth DEPTH_RENDERBUFFER_16(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH_COMPONENT16, 0 }; }
		static FrameBuffer::Depth DEPTH_RENDERBUFFER_32(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH_COMPONENT32, 0 }; }
		static FrameBuffer::Depth DEPTH_RENDERBUFFER_32F(){ return{ FrameBuffer::RenderBuffer, GL_DEPTH_COMPONENT32F, 0 }; }
		static FrameBuffer::Depth DEPTH_RENDERBUFFER_CUSTOM(GLuint name){ return{ FrameBuffer::RenderBuffer, 0, name }; }
		//Stencil
		static FrameBuffer::Stencil STENCIL_DISABLED(){ return{ FrameBuffer::Disabled }; }
		static FrameBuffer::Stencil STENCIL_TEXTURE(){ return{ FrameBuffer::Texture, 0 }; }
		static FrameBuffer::Stencil STENCIL_RENDERBUFFER(){ return{ FrameBuffer::RenderBuffer, 0 }; }
	};
};
#endif