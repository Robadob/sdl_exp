#ifndef __TextureCubeMap_h__
#define __TextureCubeMap_h__
#include "Texture.h"
#include <unordered_map>
#include <glm/vec2.hpp>

#define CUBE_MAP_FACE_COUNT 6
class TextureCubeMap : public Texture
{
public:
	static const char *SKYBOX_PATH;
    struct CubeMapParts
    {
        CubeMapParts(GLenum target, const char *name)
            : target(target), name(name) {}
        GLenum target;
        const char *name;
    };
	static const CubeMapParts FACES[CUBE_MAP_FACE_COUNT];

	/**
	* Returns a std::shared_ptr<TextureCubeMap> from the specified directory
	* @param filepath The path to the image to be loaded
	* @param options A bitmask of options which correspond to various GL texture options
	* @param skipCache If false the returned Texture2D will be added to or loaded from the cache
	*/
	static std::shared_ptr<TextureCubeMap> load(const std::string &filepath, const unsigned long long options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, bool skipCache = false);
	static bool isCached(const std::string &filePath);
	TextureCubeMap(const TextureCubeMap& b);
	TextureCubeMap(const TextureCubeMap&& b) = delete;
	TextureCubeMap& operator= (const TextureCubeMap& b) = delete;
	TextureCubeMap& operator= (const TextureCubeMap&& b) = delete;
	bool isBound() const override;
private:
	TextureCubeMap(std::shared_ptr<SDL_Surface> image[CUBE_MAP_FACE_COUNT], const std::string reference, const unsigned long long options);
	static GLuint genTextureUnit();	
	static void purgeCache(const std::string &filePath);
	static std::shared_ptr<TextureCubeMap> loadFromCache(const std::string &filePath);
	static std::unordered_map<std::string, std::weak_ptr<TextureCubeMap>> cache;
	const glm::uvec2 faceDimensions;
};
#endif