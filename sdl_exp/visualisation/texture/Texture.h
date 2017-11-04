#ifndef __Texture_h__
#define __Texture_h__

#include "../util/GLcheck.h"
#include <SDL/SDL_image.h>
#include <string>
#include <memory>
#include <glm/vec2.hpp>

/**
 * Shell texture class providing various utility methods for subclasses
 * @note This class cannot be directly instantiated
 */
class Texture
{
public:
	/**
	 * This structure holds the necessary Enums for calling various OpenGL texture funtions
	 * format, internal format and type, see docs of glTexImage2D() or similar for explanation
	 * pixelSize is the size of a single pixel in bytes, this is included for convenience (and accuracy)
	 */
	struct Format
	{
		Format(GLenum format, GLenum internalFormat, size_t pixelSize, GLenum type)
			: format(format), internalFormat(internalFormat), type(type), pixelSize(pixelSize)
		{ }
		bool operator==(const Format& other) const
		{
			return 
				other.format == this->format&&
				other.internalFormat == this->internalFormat&&
				other.type == this->type;
		}
		const GLenum format;
		const GLenum internalFormat;
		const GLenum type;
		const size_t pixelSize;//inBytes
	};
	/**
	 * Options
	 */
	//Toggles anistropic filtering, this is good at steep angles
	static const unsigned long long DISABLE_ANISTROPIC_FILTERING;
	//Selects the minifying filter, used when the texture is being downscaled
	static const unsigned long long FILTER_MIN_NEAREST;
	static const unsigned long long FILTER_MIN_LINEAR;
	static const unsigned long long FILTER_MIN_NEAREST_MIPMAP_NEAREST;
	static const unsigned long long FILTER_MIN_LINEAR_MIPMAP_NEAREST;
	static const unsigned long long FILTER_MIN_NEAREST_MIPMAP_LINEAR;
	static const unsigned long long FILTER_MIN_LINEAR_MIPMAP_LINEAR;
	//Selects the magnifying filter, used when the texture is being upscaled
	static const unsigned long long FILTER_MAG_LINEAR;
	static const unsigned long long FILTER_MAG_NEAREST;
	//Selects how texture coordinates are wrapped
	static const unsigned long long WRAP_REPEAT;
	static const unsigned long long WRAP_CLAMP_TO_EDGE;
	static const unsigned long long WRAP_CLAMP_TO_BORDER;
	static const unsigned long long WRAP_MIRRORED_REPEAT;
	static const unsigned long long WRAP_MIRROR_CLAMP_TO_EDGE;
	//Toggle's the use of mip maps (4 levels are used with mipmaps, this is not currently changeable)
	static const unsigned long long DISABLE_MIPMAP;
	/**
	 * @return The GLenum representing the type of texture, e.g. GL_TEXTURE_2D, GL_TEXTURE_CUBEMAP, GL_TEXTURE_BUFFER
	 */
	GLenum getType() const { return type; }
	/**
	 * @return The unique name given to the texture by OpenGL (allocated by glGenTextures())
	 */
	GLenum getName() const { return glName; }
	/**
	 * @return The texture unit we have assigned to this texture
	 * @note Texture's of different type's may share this texture unit, using the appropriate sampler in shaders will select the correct texture
	 * @note Currently we are not set up to have more textures of any type than the number of unique units (~92)
	 */
	GLenum getTextureUnit() const { return textureUnit; }
	/**
	 * @return The name assigned to the texture
	 * @note For textures loaded from file, this will be the file path
	 * @note For raw textures this will simply be the string of the enum returned by getType()
	 */
	std::string getReference() const { return reference; }
	/**
	 * @return Whether the texture is the currently bound texture of it's given type to the assigned texture unit
	 */
	virtual bool isBound() const = 0;
	/**
	 * Binds the texture to it's currently assigned texture unit
	 */
	void bind() const;
	/**
	 * Returns whether the anistropic filtering option is enabled according to the selected options
	 */
	bool enableAnistropicOption() const;
	/**
	 * Returns the GLenum of the selected minifying filter option
	 */
	GLenum filterMinOption() const;
	/**
	 * Returns the GLenum of the selected magnifying filter option
	 */
	GLenum filterMagOption() const;
	/**
	 * Returns the GLenum of the selected texture wrap option
	 */
	GLenum wrapOption() const;
	/**
	 * Returns whether mipmapping is enabled according to the selected options
	 */
	bool enableMipMapOption() const;
	/**
	 * @param fileExtension A given string of a file extension
	 * @return Wether the image loader is capable of handling the given extension
	 * @note Indifferent to case-sensitivity and the inclusion of initial '.'
	 */
	static bool supportsExtension(const std::string &fileExtension);
	/**
	 * Virtual destructor, deallocate the OpenGL texture
	 * @note Does not deallocate external texture's
	 */
	virtual ~Texture();
protected:
	/**
	 * Remove all copy/move/assignment contructors
	 * Subclasses should call the regular constructor from any copy constructors.
	 */
	Texture(const Texture& b) = delete;
	Texture(const Texture&& b) = delete;
	Texture& operator= (const Texture& b) = delete;
	Texture& operator= (const Texture&& b) = delete;
	/**
	 * @param type The texture type e.g. GL_TEXTURE_2D
	 * @param textureUnit The chosen texture unit to be used by the texture
	 * @param format The internal format specifiers of the texture
	 * @param reference A string to identifyy the texture
	 * @param options The filtering/wrapping/mipmap options for how the texture should be handled
	 * @param glName If non 0 this will force the texture to use an external GL texture, rather than allocate it's own
	 */
	Texture(GLenum type, GLuint textureUnit, const Format &format, const std::string &reference, unsigned long long options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, GLuint glName=0);
	void applyOptions();
	/**
	 * Copies a texture from an SDL_Surface into the host object's GL texture
	 * @param image The SDL_Surface to upload to the GL texture
	 * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
	 * @note This function uses glTexStorage2D so cannot be called multiple times with the same target
	 */
	void allocateTexture(std::shared_ptr<SDL_Surface> image, GLenum target = 0);
	void allocateTexture(const void *data, const glm::uvec2 &dimensions, GLenum target = 0);
	void setTexture(const void *data, const glm::uvec2 &dimensions, glm::ivec2 offset = glm::ivec2(0), GLenum target = 0);
	const GLenum type;
	const GLuint glName;	
	const GLuint textureUnit;
	const std::string reference;
	const Format format;
	const unsigned long long options;
	/**
	 * Returns the first image found at the provided path
	 * This method attempts all the suffices stored in Texture::IMAGE_EXTS
	 * @imagePath Path to search for images
	 */
	static std::shared_ptr<SDL_Surface> findLoadImage(const std::string &imagePath);
	static std::shared_ptr<SDL_Surface> loadImage(const std::string &imagePath, bool flipVertical=true, bool silenceErrors = false);
	/**
	 * We use this when loading an image with SDL_Image to invert the image rows.
	 * This is because most image formats label images with the origin in the top left corner
	 * Whereas glTexImage2D expects the origin to be in the bottom left corner.
	 * We could handle this by negating Y when using texcoords, however this better allows us to standardise shaders
	 * @param img The SDL_Surface to be flipped
	 * @return true on success, else failure
	 * @note original source: http://www.gribblegames.com/articles/game_programming/sdlgl/invert_sdl_surfaces.html
	 */
	static bool flipRows(std::shared_ptr<SDL_Surface> img);
	static Format getFormat(std::shared_ptr<SDL_Surface> image);
	static const char *IMAGE_EXTS[5];
private:
	static GLuint genTexName();
	const bool externalTex;
};

#endif //ifndef __Texture_h__