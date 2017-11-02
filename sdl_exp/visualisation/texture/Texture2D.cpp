#include "Texture2D.h"
#include <cassert>
#include <glm/gtx/component_wise.hpp>

/**
* Constructors
*/
Texture2D::Texture2D(std::shared_ptr<SDL_Surface> image, const std::string reference, const unsigned long long options)
	: Texture(GL_TEXTURE_2D, genTextureUnit(), getFormat(image), reference, options)
	, dimensions(image->w, image->h)
{
	assert(image);
	fillTexture(image);
	applyOptions();
}
/**
 * Copy/Assignment handling
 */
Texture2D::Texture2D(const Texture2D& b)
	: Texture(b.type, genTextureUnit(), b.format, std::string(b.reference).append("!"), b.options)
	, dimensions(b.dimensions)
{
	//Negate the need for padding
	GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	//Copy the actual texture data
	const size_t dataSize = format.pixelSize*compMul(dimensions);
	unsigned char *tex = (unsigned char *)malloc(dataSize);
	GL_CALL(glBindTexture(b.type, b.glName));
	GL_CALL(glGetTextureImage(b.type, 0, format.format, format.type, dataSize, tex));
	GL_CALL(glBindTexture(type, glName));
	GL_CALL(glTexStorage2D(type, enableMipMapOption() ? 4 : 1, format.internalFormat, dimensions.x, dimensions.y));//Must not be called twice on the same gl tex
	GL_CALL(glTexSubImage2D(type, 0, 0, 0, dimensions.x, dimensions.y, format.format, format.type, tex));
	free(tex);
	//Reset the need for padding
	GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 4));
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	applyOptions();
}
/**
 * Cache handling
 */
std::shared_ptr<Texture2D> Texture2D::loadFromCache(const std::string &filePath)
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
std::shared_ptr<Texture2D> Texture2D::load(const std::string &filePath, const unsigned long long options, bool skipCache)
{
	//Attempt from cache
	std::shared_ptr<Texture2D> rtn;
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
			rtn = std::shared_ptr<Texture2D>(new Texture2D(image, filePath, options),
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
	if (texUnit < (GLuint)maxUnits)
	{
		texUnit = 1;
		fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D, enable texture switching");
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