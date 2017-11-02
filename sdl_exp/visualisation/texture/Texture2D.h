#ifndef __Texture2D_h__
#define __Texture2D_h__

#include "../util/GLcheck.h"
#include <SDL/SDL_image.h>
#include <memory>
#include <unordered_map>
#include <locale>
#include <algorithm>
#include "Texture.h"
#include <glm/vec2.hpp>
class TextureCore;
/*
Shell texture class providing various utility methods for subclasses
@note This class cannot be directly instantiated
*/
class Texture2D : public Texture
{
public:
	/**
	 * Returns a std::shared_ptr<Texture2D> of the specified file
	 * @param filepath The path to the image to be loaded
	 * @param options A bitmask of options which correspond to various GL texture options
	 * @param skipCache If false the returned Texture2D will be added to or loaded from the cache
	 */
	static std::shared_ptr<Texture2D> load(const std::string &filepath, const unsigned long long options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, bool skipCache = false);
	static bool isCached(const std::string &filePath);
	Texture2D(const Texture2D& b);
	Texture2D(const Texture2D&& b) = delete;
	Texture2D& operator= (const Texture2D& b) = delete;
	Texture2D& operator= (const Texture2D&& b) = delete;
	glm::uvec2 getDimensions() const { return dimensions; }
	unsigned int getWidth() const { return dimensions.x; }
	unsigned int getHeight() const { return dimensions.y; }
	bool isBound() const override;
private:
	Texture2D(std::shared_ptr<SDL_Surface> image, const std::string reference, const unsigned long long options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
	static GLuint genTextureUnit();
	static void purgeCache(const std::string &filePath);
	static std::shared_ptr<Texture2D> loadFromCache(const std::string &filePath);
	static std::unordered_map<std::string, std::weak_ptr<Texture2D>> cache;
	const glm::uvec2 dimensions;
};

#endif //ifndef __Texture2D_h__