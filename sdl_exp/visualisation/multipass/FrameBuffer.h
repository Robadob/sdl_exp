#ifndef __FrameBuffer_h__
#define __FrameBuffer_h__

#include "glm/glm.hpp"
#include "FrameBufferAttachment.h"
#include <map>
#include "../interface/FBuffer.h"
#include <unordered_set>

/**
 * https://open.gl/framebuffers
 * @todo Include glDrawbuffers call at use?
 * @todo Improve stencilbuffer (control when writing/clearing/using) //https://en.wikipedia.org/wiki/Stencil_buffer
 * @todo MultisampleFrameBuffer subclass (all attachments must be the same type (tex vs renderbuffer) + samples ct)
 */
class FrameBuffer : public FBuffer
{
public:
    ////////////////////////////
    //  Scaling FrameBuffers  //
    ////////////////////////////
    FrameBuffer(FrameBufferAttachment color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(FrameBufferAttachment color, FrameBufferAttachment depthstencil,unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depthstencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    ////////////////////////////
    //   Fixed FrameBuffers   //
    ////////////////////////////
    FrameBuffer(glm::uvec2 dimensions, FrameBufferAttachment color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, FrameBufferAttachment color, FrameBufferAttachment depthstencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depthstencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
	~FrameBuffer();
	/**
	 * @return Whether the framebuffer is 'complete' and ready to be rendered to
	 */
	bool isValid() const;
	/**
	 * Resizes the internal images according to the specifed dimensions and the internal scaling factor
	 * @param width The new viewport width
	 * @param height The new viewport height
	 */
	void resize(int width, int height) override final;
	/**
	 * Binds the framebuffer
	 * @return True if the framebuffer is 'complete' and was bound
	 */
	bool use() override final;
	/**
	 * @return The name of the contained GL_FRAMEBUFFER
	 */
	GLuint getFrameBufferName() override { return name; };
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
    /**
     * @return The number of samples
     * @note 0 Means that multisampling for the FrameBuffer is disabled
     */
    unsigned int getSampleCount(){ return samples; }
    /**
    * @return The current dimensions of the FrameBuffer
    * @note If this is a scaling FrameBuffer this value may change over time
    */
    glm::uvec2 getDimensions(){ return dimensions; }
    /**
    * Sets whether the Framebuffer is to be automatically cleared before use
    */
    void setDoClear(bool doClear){ this->doClear = doClear; }
    /**
    * @return Whether the Framebuffer is to be automatically cleared before use
    */
    bool getDoClear(){ return doClear; }
    /**
    * Sets the clear color to be used
    */
    void setClearColor(glm::vec3 doClear){ this->clearColor = clearColor; }
    /**
    * @return The clear color to be used
    */
    glm::vec3 getClearColor(){ return clearColor; }

private:
    inline GLenum GL_TEXTURE_TYPE() const { return samples == 0 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE; }
	/**
	 * Internal constructor, triggered by public ones
	 */
    FrameBuffer(std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depth, FrameBufferAttachment stencil, FrameBufferAttachment depthstencil, float scale, glm::uvec2 dimensions, unsigned int samples, glm::vec3 clearColor, bool doClear);
    /**
     * Does all the heavy lifting for generating attachments
     */
    void makeAttachment(const FrameBufferAttachment &attachmentConfig, GLenum attachPoint, GLuint *texNameOut) const;
    /**
	 * Generates and resizes all attachments
	 */
    void makeAttachments();
	/**
	 * Scale of images held by this framebuffer, with respect to the viewport
	 * If the scale <= 0, Then the framebuffer has fixed dimensions
	 */
	const float scale;
	/**
	 * Current dimensions of images held by this framebuffer
	 */
	glm::uvec2 dimensions;
    /**
     * The number of samples used, if the value is 0 multisampling will be disabled
     */
    unsigned int samples;
	/**
	 * The name of the framebuffer, as set by glGenFramebuffers()
	 */
    GLuint name;
	/**
	 * The clear colour to clear the frame with prior to rendering
	 */
	glm::vec3 clearColor;
	/**
	 * Whether to clear the frame prior to rendering
	 */
	bool doClear;
    struct ConfNamePair
    {
        ConfNamePair(FrameBufferAttachment fbaConf)
            : conf(fbaConf), texName(0){ }
        /**
         * The frame buffer attachment configuration
         */
        const FrameBufferAttachment conf;
        /**
         * The GL texture/renderbuffer name for the attachment (as returned by glGenTextures() or glGenRenderbuffers())
         */
        GLuint texName;
    };
	/**
	 * Config for each color attachment
	 * Key: Color attachment point
	 * Value: Color attachment configuration/name
	 */
    std::map<GLuint, ConfNamePair> colors;
	/**
	 * Config for the depth stencil attachment
	 */
    ConfNamePair depthStencil;
    /**
    * Config for the stencil attachment
    */
    ConfNamePair depth;
	/**
	 * Config for the stencil attachment
	 */
     ConfNamePair stencil;
    /**
     * Notifies the framebuffer which color attachments are used
     * @note This method can only be called once per framebuffer
     */
	void setDrawBuffers();
};
#endif