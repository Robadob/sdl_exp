#include "Texture2D.h"
#include <cassert>
#include <glm/gtx/component_wise.hpp>

const char *Texture2D::RAW_TEXTURE_FLAG = "Texture2D";
std::unordered_map<std::string, std::weak_ptr<const Texture2D>> Texture2D::cache;

/**
* Constructors
*/
Texture2D::Texture2D(std::shared_ptr<SDL_Surface> image, const std::string reference, const unsigned long long options)
	: Texture(GL_TEXTURE_2D, genTextureUnit(), getFormat(image), reference, options)
	, dimensions(image->w, image->h)
    , immutable(true)
{
	assert(image);
	allocateTextureImmutable(image);
	applyOptions();
}
Texture2D::Texture2D(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data, const unsigned long long &options)
	: Texture(GL_TEXTURE_2D, genTextureUnit(), format, RAW_TEXTURE_FLAG, options)
	, dimensions(dimensions)
    , immutable(false)
{
    allocateTextureMutable(dimensions, data);
	applyOptions();
}
/**
 * Copy/Assignment handling
 */
Texture2D::Texture2D(const Texture2D& b)
	: Texture(b.type, genTextureUnit(), b.format, std::string(b.reference).append("!"), b.options)
	, dimensions(b.dimensions)
    , immutable(false)
{
    allocateTextureMutable(dimensions, nullptr);
    GL_CALL(glCopyImageSubData(
        b.glName, b.type, 0, 0, 0, 0,
        glName, type, 0, 0, 0, 0, 
        b.dimensions.x, b.dimensions.y, 0));
	applyOptions();
}
/**
 * Factory
 */
std::shared_ptr<Texture2D> Texture2D::make(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data, const unsigned long long &options)
{
	return std::shared_ptr<Texture2D>(new Texture2D(dimensions, format, data, options));
}
std::shared_ptr<Texture2D> Texture2D::make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned long long &options)
{
	return make(dimensions, format, nullptr, options);
}
/**
 * Cache handling
 */
std::shared_ptr<const Texture2D> Texture2D::loadFromCache(const std::string &filePath)
{
	auto a = cache.find(filePath);
	if (a != cache.end())
	{
        if (auto b = a->second.lock())
		{
			return b;
		}
		//Weak pointer has expired, erase record 
		//This should be redundant, if custom deleter has been used
		cache.erase(a);
	}
	return nullptr;
}
std::shared_ptr<const Texture2D> Texture2D::load(const char * filePath, const unsigned long long options, bool skipCache)
{
    if (filePath)
        return load(std::string(filePath), options, skipCache);
    return std::shared_ptr <const Texture2D>();
}
std::shared_ptr<const Texture2D> Texture2D::load(const std::string &filePath, const unsigned long long options, bool skipCache)
{
	//Attempt from cache
	std::shared_ptr<const Texture2D> rtn;
	if (!skipCache)
	{
		rtn = loadFromCache(filePath);
	}
	//Load using loader
	if (!rtn)
	{
		auto image = loadImage(filePath);
		if (image)
		{
			rtn = std::shared_ptr<const Texture2D>(new Texture2D(image, filePath, options),
				[&](Texture2D *ptr){//Custom deleter, which purges cache of item
					Texture2D::purgeCache(ptr->getReference());
					delete ptr;
				});
		}
	}
	//If we've loaded something, store in cache
	if (rtn&&!skipCache)
	{
		cache.emplace(filePath, rtn);
	}
	return rtn;
}
bool Texture2D::isCached(const std::string &filePath)
{
	auto a = cache.find(filePath);
	if (a != cache.end())
	{
		if (a->second.lock())
			return true;
		//Weak pointer has expired, erase record 
		//This should be redundant, if custom deleter has been used
		cache.erase(a);
	}
	return false;
}
void Texture2D::purgeCache(const std::string &filePath)
{
	auto a = cache.find(filePath);
	if (a != cache.end())
	{
		//Erase record
		cache.erase(a);
	}
}
void Texture2D::resize(const glm::uvec2 &dimensions, void *data, size_t size)
{
    if (immutable)
    {
        throw std::exception("Textures loaded from image are immutable and cannot be changed.\n");
        return;
    }
    if (data&&size)
		assert(size == format.pixelSize*compMul(dimensions));
	this->dimensions = dimensions;
	allocateTextureMutable(dimensions, data);
	//If image data has been updated, regen mipmap
	if (data)
	{
		GL_CALL(glBindTexture(type, glName));
		GL_CALL(glGenerateMipmap(type));
		GL_CALL(glBindTexture(type, 0));
	}
}
void Texture2D::setTexture(void *data, size_t size)
{
	Texture2D::setSubTexture(data, this->dimensions, glm::ivec2(0), size);
}
void Texture2D::setSubTexture(void *data, glm::uvec2 dimensions, glm::ivec2 offset, size_t size)
{
	if (immutable)
	{
        throw std::exception("Textures loaded from image are immutable and cannot be changed.\n");
		return;
	}
	if (size)
		assert(size == format.pixelSize*compMul(dimensions));
	this->dimensions = dimensions;
	Texture::setTexture(data, dimensions, offset);
	//If image data has been updated, regen mipmap
	if (data)
	{
		GL_CALL(glBindTexture(type, glName));
		GL_CALL(glGenerateMipmap(type));
		GL_CALL(glBindTexture(type, 0));
	}
}
/**
 * Required methods for handling texture units
 */
GLuint Texture2D::genTextureUnit()
{
	static GLuint texUnit = 1;
	GLint maxUnits;
	GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));//192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
	assert(texUnit < (GLuint)maxUnits);
#endif
	if (texUnit >= (GLuint)maxUnits)
	{
		texUnit = 1;
		fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D, enable texture switching.\n");
		//If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
		//Possibly even notifying it of duplicate units
	}
	return texUnit++;
}
bool Texture2D::isBound() const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0+textureUnit));
	GLint whichID;
	GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID));
	return whichID == glName;
}
//Comment out this include if not making use of GaussianBlur
#include "../shader/GaussianBlur.h"
#ifdef __GaussianBlur_h__
void GaussianBlur::blurR32F(std::shared_ptr<Texture2D> inTex, std::shared_ptr<Texture2D> outTex)
{
#ifdef _DEBUG
	assert(inTex->getDimensions() == outTex->getDimensions());
#endif
	blurR32F(inTex->getName(), outTex->getName(), inTex->getDimensions());
}
#endif