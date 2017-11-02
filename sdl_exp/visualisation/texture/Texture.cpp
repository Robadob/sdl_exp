#include "Texture.h"
#include <cassert>
#include <algorithm>

//Ensure these remain lowercase without prepended '.'
const char* Texture::IMAGE_EXTS[] = {
	"tga",
	"png",
	"bmp",
	"jpg",
	"webp"
};

//Filter Ext Options
const unsigned long long Texture::DISABLE_ANISTROPIC_FILTERING = 1ull << 0;
bool Texture::enableAnistropicOption() const
{
	return  !((options & DISABLE_ANISTROPIC_FILTERING) == DISABLE_ANISTROPIC_FILTERING);
}

//Filter Min Options
const unsigned long long Texture::FILTER_MIN_NEAREST				= 1ull << 1;
const unsigned long long Texture::FILTER_MIN_LINEAR					= 1ull << 2;
const unsigned long long Texture::FILTER_MIN_NEAREST_MIPMAP_NEAREST = 1ull << 3;
const unsigned long long Texture::FILTER_MIN_LINEAR_MIPMAP_NEAREST	= 1ull << 4;
const unsigned long long Texture::FILTER_MIN_NEAREST_MIPMAP_LINEAR	= 1ull << 5;//GL_Default value
const unsigned long long Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR	= 1ull << 6;
GLenum Texture::filterMinOption() const
{
	GLenum rtn = GL_INVALID_ENUM;
	if ((options & FILTER_MIN_NEAREST) == FILTER_MIN_NEAREST)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_NEAREST;
	}
	if ((options & FILTER_MIN_LINEAR) == FILTER_MIN_LINEAR)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_LINEAR;
	}
	if ((options & FILTER_MIN_NEAREST_MIPMAP_NEAREST) == FILTER_MIN_NEAREST_MIPMAP_NEAREST)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_NEAREST_MIPMAP_NEAREST;
	}
	if ((options & FILTER_MIN_LINEAR_MIPMAP_NEAREST) == FILTER_MIN_LINEAR_MIPMAP_NEAREST)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_LINEAR_MIPMAP_NEAREST;
	}
	if ((options & FILTER_MIN_NEAREST_MIPMAP_LINEAR) == FILTER_MIN_NEAREST_MIPMAP_LINEAR)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_NEAREST_MIPMAP_LINEAR;
	}
	if ((options & FILTER_MIN_LINEAR_MIPMAP_LINEAR) == FILTER_MIN_LINEAR_MIPMAP_LINEAR)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_LINEAR_MIPMAP_LINEAR;
	}
	return rtn == GL_INVALID_ENUM ? GL_NEAREST_MIPMAP_LINEAR : rtn;//If none specified, return GL default
}

//Filter Mag Options
const unsigned long long Texture::FILTER_MAG_LINEAR		= 1ull << 7;//GL_Default value
const unsigned long long Texture::FILTER_MAG_NEAREST	= 1ull << 8;
GLenum Texture::filterMagOption() const
{
	GLenum rtn = GL_INVALID_ENUM;
	if ((options & FILTER_MAG_LINEAR) == FILTER_MAG_LINEAR)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_LINEAR;
	}
	if ((options & FILTER_MAG_NEAREST) == FILTER_MAG_NEAREST)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_NEAREST;
	}
	return rtn == GL_INVALID_ENUM ? GL_LINEAR : rtn;//If none specified, return GL default
}

//Wrap Options
const unsigned long long Texture::WRAP_REPEAT				= 1ull << 9;//GL_Default value
const unsigned long long Texture::WRAP_CLAMP_TO_EDGE		= 1ull << 10;
const unsigned long long Texture::WRAP_CLAMP_TO_BORDER		= 1ull << 11;
const unsigned long long Texture::WRAP_MIRRORED_REPEAT		= 1ull << 12;
const unsigned long long Texture::WRAP_MIRROR_CLAMP_TO_EDGE = 1ull << 13;
GLenum Texture::wrapOption() const
{
	GLenum rtn = GL_INVALID_ENUM;
	if ((options & WRAP_REPEAT) == WRAP_REPEAT)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_REPEAT;
	}
	if ((options & WRAP_CLAMP_TO_EDGE) == WRAP_CLAMP_TO_EDGE)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_CLAMP_TO_EDGE;
	}
	if ((options & WRAP_CLAMP_TO_BORDER) == WRAP_CLAMP_TO_BORDER)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_CLAMP_TO_BORDER;
	}
	if ((options & WRAP_MIRRORED_REPEAT) == WRAP_MIRRORED_REPEAT)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_MIRRORED_REPEAT;
	}
	if ((options & WRAP_MIRROR_CLAMP_TO_EDGE) == WRAP_MIRROR_CLAMP_TO_EDGE)
	{
		assert(rtn == GL_INVALID_ENUM);//Invalid bitmask, multiple conflicting options passed
		rtn = GL_MIRROR_CLAMP_TO_EDGE;
	}
	return rtn == GL_INVALID_ENUM ? GL_REPEAT : rtn;//If none specified, return GL default
}

//MipMap Options
const unsigned long long Texture::DISABLE_MIPMAP = 1ull << 14;
bool Texture::enableMipMapOption() const
{
	return  !((options & DISABLE_MIPMAP) == DISABLE_MIPMAP);
}

//Constructors
Texture::Texture(GLenum type, GLuint textureUnit, const Format &format, const std::string &reference, unsigned long long options, GLuint glName)
	: type(type)
	, glName(glName==0?genTexName():glName)
	, textureUnit(textureUnit)
	, reference(reference)
	, format(format)
	, options(options)
	, externalTex(glName!=0)
{
	assert(textureUnit != 0);//We reserve texture unit 0 for texture commands, because if we bind a texture to change settings we would knock the desired one out of the unit
	bind();
}
Texture::~Texture()
{
	if (!externalTex)
	{
		GL_CALL(glDeleteTextures(1, &glName));
	}
}

//General image loading utils
GLuint Texture::genTexName()
{
	GLuint texName = 0;
	GL_CALL(glGenTextures(1, &texName));
	return texName;
}
void Texture::applyOptions()
{
	GL_CALL(glBindTexture(type, glName));
	if (enableAnistropicOption())
	{//Anistropic filtering (improves texture sampling at steep angle, especially visible with tiling patterns)
		GLfloat fLargest;
		GL_CALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest));
		GL_CALL(glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest));
	}

	GL_CALL(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filterMagOption()));
	GL_CALL(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filterMinOption()));

	GLenum wrapMode =wrapOption();
	GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapMode));
	GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode));
	GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapMode));

	if (enableMipMapOption())
	{
		GL_CALL(glGenerateMipmap(type));
	}
	GL_CALL(glBindTexture(type, 0));
}
std::shared_ptr<SDL_Surface> Texture::findLoadImage(const std::string &imagePath)
{
	//Attempt without appending extension
	std::shared_ptr<SDL_Surface> image = loadImage(imagePath);
	for (int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*) && !image; i++)
	{
		image = loadImage(std::string(imagePath).append(".").append(IMAGE_EXTS[i]), true, false);
	}
	return image;
}
std::shared_ptr<SDL_Surface> Texture::loadImage(const std::string &imagePath, bool flipVertical, bool silenceErrors)
{
	std::shared_ptr<SDL_Surface> image = std::shared_ptr<SDL_Surface>(IMG_Load(imagePath.c_str()), SDL_FreeSurface);
	if (!image)
	{
		if (!silenceErrors)
			fprintf(stderr, "%s\n", IMG_GetError());
		return std::shared_ptr<SDL_Surface>();
	}
	//This was used before I bothered to write in support for all GL'able formats
	//else if (image->format->BytesPerPixel != 3 && image->format->BytesPerPixel != 4)
	//{
	//	if (!silenceErrors)
	//		fprintf(stderr, "'%s': Textures must be 3 or 4 channel with 8 bits per colour.\n", imagePath.c_str());
	//	return std::shared_ptr<SDL_Surface>();
	//}
	//else if (image->format->Rshift>image->format->Bshift)
	//{   //If the texture is BGR order rather than RGB order, switch bytes
	//	SDL_PixelFormat desiredFormat;
	//	memcpy(&desiredFormat, image->format, sizeof(SDL_PixelFormat));
	//	desiredFormat.Bloss = image->format->Rloss;
	//	desiredFormat.Bmask = image->format->Rmask;
	//	desiredFormat.Bshift = image->format->Rshift;
	//	desiredFormat.Rloss = image->format->Bloss;
	//	desiredFormat.Rmask = image->format->Bmask;
	//	desiredFormat.Rshift = image->format->Bshift;
	//	if (image->format->BytesPerPixel == 3)
	//		desiredFormat.format = SDL_PIXELFORMAT_RGB888;
	//	else
	//		desiredFormat.format = SDL_PIXELFORMAT_RGBA8888;
	//	image = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(image.get(), &desiredFormat, 0), SDL_FreeSurface);
	//}
	SDL_ClearError();//Clear any img errors
	if (image&&flipVertical)
		flipRows(image);
	return image;
}

bool Texture::flipRows(std::shared_ptr<SDL_Surface> img)
{
	if (!img)
	{
		SDL_SetError("Surface is NULL");
		return false;
	}
	int pitch = img->pitch;
	int height = img->h;
	void* image_pixels = img->pixels;
	int index;
	void* temp_row;
	int height_div_2;

	temp_row = (void *)malloc(pitch);
	if (NULL == temp_row)
	{
		SDL_SetError("Not enough memory for image inversion");
		return false;
	}
	//if height is odd, don't need to swap middle row
	height_div_2 = (int)(height * .5);
	for (index = 0; index < height_div_2; index++) 	{
		//uses string.h
		memcpy((Uint8 *)temp_row,
			(Uint8 *)(image_pixels)+
			pitch * index,
			pitch);

		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * index,
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			pitch);
		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			temp_row,
			pitch);
	}
	free(temp_row);
	return true;
}
void Texture::fillTexture(std::shared_ptr<SDL_Surface> image, GLenum target)
{
	GLint internalFormat = image->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA;

	GL_CALL(glBindTexture(type, glName));
	//If the image is stored with a pitch different to width*bytes per pixel, temp change setting
	if (image->pitch / image->format->BytesPerPixel != image->w)
	{
		GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, image->pitch / image->format->BytesPerPixel));
	}
	GLint sizedIF = image->format->BytesPerPixel == 3 ? GL_RGB8 : GL_RGBA8;
	GL_CALL(glTexStorage2D(type, enableMipMapOption() ? 4 : 1, sizedIF, image->w, image->h));//Must not be called twice on the same gl tex
	GL_CALL(glTexSubImage2D(type, 0, 0, 0, image->w, image->h, internalFormat, GL_UNSIGNED_BYTE, image->pixels));
	//Disable custom pitch
	if (image->pitch / image->format->BytesPerPixel != image->w)
	{
		GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
	}
	GL_CALL(glBindTexture(type, 0));
}

bool Texture::supportsExtension(const std::string &fileExtension)
{
	std::string _fileExtension;
	std::transform(fileExtension.begin(), fileExtension.end(), _fileExtension.begin(), ::tolower);
	//Compare each extension with and without . prepended
	for (unsigned int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*); ++i)
	{
		if (_fileExtension.compare(IMAGE_EXTS[i]) == 0)
			return true;
		if (_fileExtension.compare(std::string(".") + IMAGE_EXTS[i]) == 0)
			return true;
	}
	return false;
};
void Texture::bind() const
{
#ifdef _DEBUG
	if (isBound())
		return;
#endif
	GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
	GL_CALL(glBindTexture(type, glName));
	//Always return to Tex0 for doing normal texture work
	GL_CALL(glActiveTexture(GL_TEXTURE0));
}

Texture::Format Texture::getFormat(std::shared_ptr<SDL_Surface> image)
{
	assert(image);
	switch (image->format->format)
	{
	case SDL_PIXELFORMAT_RGB332:
		return Format(GL_RGB, GL_R3_G3_B2, 1, GL_UNSIGNED_BYTE_3_3_2);
	case SDL_PIXELFORMAT_RGB444:
		return Format(GL_RGB, GL_RGB4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
	case SDL_PIXELFORMAT_BGR555:
		return Format(GL_BGR, GL_RGB5, 2, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_RGB555:
		return Format(GL_RGB, GL_RGB5, 2, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_ABGR4444:
		return Format(GL_ABGR_EXT, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
	case SDL_PIXELFORMAT_RGBA4444:
		return Format(GL_RGBA, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
	case SDL_PIXELFORMAT_BGRA4444:
		return Format(GL_BGRA, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
	case SDL_PIXELFORMAT_BGRA5551:
		return Format(GL_BGRA, GL_RGB5_A1, 2, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_RGBA5551:
		return Format(GL_RGBA, GL_RGB5_A1, 2, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_RGB565:
		return Format(GL_RGB, GL_RGB, 2, GL_UNSIGNED_SHORT_5_6_5);
	case SDL_PIXELFORMAT_BGR565:
		return Format(GL_BGR, GL_RGB, 2, GL_UNSIGNED_SHORT_5_6_5);
	case SDL_PIXELFORMAT_RGB24:
		return Format(GL_RGB, GL_RGB8, 3, GL_UNSIGNED_BYTE);//Is this correct?
	case SDL_PIXELFORMAT_BGR24:
		return Format(GL_BGR, GL_RGB8, 3, GL_UNSIGNED_BYTE);//Is this correct?
	case SDL_PIXELFORMAT_RGB888:
		return Format(GL_RGB, GL_RGB8, 3, GL_UNSIGNED_BYTE);//Is this correct?
	case SDL_PIXELFORMAT_BGR888:
		return Format(GL_BGR, GL_RGB8, 3, GL_UNSIGNED_BYTE);//Is this correct?
	case SDL_PIXELFORMAT_RGBA8888:
		return Format(GL_RGBA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
	case SDL_PIXELFORMAT_ABGR8888:
		return Format(GL_ABGR_EXT, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
	case SDL_PIXELFORMAT_BGRA8888:
		return Format(GL_BGRA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
	//Possible if we bother to write reorder to RGBA functions
	case SDL_PIXELFORMAT_ARGB4444:
		//return Format(GL_RGBA, GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
	case SDL_PIXELFORMAT_ARGB1555:
		//return Format(GL_RGBA, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_ABGR1555:
		//return Format(GL_RGBA, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
	case SDL_PIXELFORMAT_ARGB8888:
		//return Format(GL_RGBA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8);
	//Unknown
	case SDL_PIXELFORMAT_ARGB2101010:
	case SDL_PIXELFORMAT_RGBX8888:
	case SDL_PIXELFORMAT_BGRX8888:
	case SDL_PIXELFORMAT_NV21:
	case SDL_PIXELFORMAT_NV12:
	case SDL_PIXELFORMAT_YVYU:
	case SDL_PIXELFORMAT_UYVY:
	case SDL_PIXELFORMAT_YUY2:
	case SDL_PIXELFORMAT_IYUV:
	case SDL_PIXELFORMAT_YV12:
	case SDL_PIXELFORMAT_UNKNOWN:
	case SDL_PIXELFORMAT_INDEX1LSB:
	case SDL_PIXELFORMAT_INDEX1MSB:
	case SDL_PIXELFORMAT_INDEX4LSB:
	case SDL_PIXELFORMAT_INDEX4MSB:
	case SDL_PIXELFORMAT_INDEX8:
	default:
		fprintf(stderr, "Unable to handle SDL_PIXELFORMAT: %d\n", image->format->format);
		assert(false);
	}
	return Format(0, 0, 0, 0);
}
//Comment out this include if not making use of Shaders/ShaderCore
#include "../shader/ShaderCore.h"
#ifdef __ShaderCore_h__
bool ShaderCore::addTexture(const char *textureNameInShader, std::shared_ptr<Texture> texture)
{//Treat it similar to texture binding points
	return addTexture(textureNameInShader, texture->getType(), texture->getName(), texture->getTextureUnit());
}
#endif