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
	static const unsigned long long DISABLE_ANISTROPIC_FILTERING;

	static const unsigned long long FILTER_MIN_NEAREST;
	static const unsigned long long FILTER_MIN_LINEAR;
	static const unsigned long long FILTER_MIN_NEAREST_MIPMAP_NEAREST;
	static const unsigned long long FILTER_MIN_LINEAR_MIPMAP_NEAREST;
	static const unsigned long long FILTER_MIN_NEAREST_MIPMAP_LINEAR;
	static const unsigned long long FILTER_MIN_LINEAR_MIPMAP_LINEAR;

	static const unsigned long long FILTER_MAG_LINEAR;
	static const unsigned long long FILTER_MAG_NEAREST;

	static const unsigned long long WRAP_REPEAT;
	static const unsigned long long WRAP_CLAMP_TO_EDGE;
	static const unsigned long long WRAP_CLAMP_TO_BORDER;
	static const unsigned long long WRAP_MIRRORED_REPEAT;
	static const unsigned long long WRAP_MIRROR_CLAMP_TO_EDGE;

	static const unsigned long long DISABLE_MIPMAP;
	GLenum getType() const { return type; }
	GLenum getName() const { return glName; }
	GLenum getTextureUnit() const { return textureUnit; }
	std::string getReference() const { return reference; }

	virtual bool isBound() const = 0;
	void bind() const;

	bool enableAnistropicOption() const;
	GLenum filterMinOption() const;
	GLenum filterMagOption() const;
	GLenum wrapOption() const;
	bool enableMipMapOption() const;
	static bool supportsExtension(const std::string &fileExtension);
	virtual ~Texture();
protected:
	Texture(const Texture& b) = delete;//Subclasses should call the normal constructor
	Texture(const Texture&& b) = delete;
	Texture& operator= (const Texture& b) = delete;
	Texture& operator= (const Texture&& b) = delete;
	/**
	 * @param type The texture type e.g. GL_TEXTURE_2D
	 */
	Texture(GLenum type, GLuint textureUnit, const Format &format, const std::string &reference, unsigned long long options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, GLuint glName=0);
	void applyOptions();
	/**
	 * Copies a texture from an SDL_Surface into the host object's GL texture
	 * @param image The SDL_Surface to upload to the GL texture
	 * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
	 * @note This function uses glTexStorage2D so cannot be called multiple times with the same target
	 */
    void allocateTextureImmutable(std::shared_ptr<SDL_Surface> image, GLenum target = 0);
    void allocateTextureImmutable(const glm::uvec2 &dimensions, const void *data = nullptr, GLenum target = 0);
    void allocateTextureMutable(const glm::uvec2 &dimensions, const void *data = nullptr, GLenum target = 0);
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