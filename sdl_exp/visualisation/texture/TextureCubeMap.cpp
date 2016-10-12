#include "TextureCubeMap.h"
/*
The stock skybox path (used as the default cube map texture
*/
const char *TextureCubeMap::SKYBOX_PATH = "../textures/skybox/";
/*
The file name (without the file type) to face mapping used by cube maps
*/
const TextureCubeMap::CubeMapParts TextureCubeMap::FACES[] = {
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "left"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "right"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "up"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "down"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "front"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "back")
};
/*
Constructs a new cubemap texture from files in the provided directory
@param cubeMapDirectory Path to a directory that contains 6 files of the names provided in TextureCubeMap::FACES
@param uniformName The name of the uniform sampler within the shader (this defaults to the value of Texture::TEXTURE_UNIFORM_NAME)
@note The directory must end with a slash
*/
TextureCubeMap::TextureCubeMap(const char *cubemapDirectory, char *uniformName)
    :Texture(GL_TEXTURE_CUBE_MAP, cubemapDirectory, uniformName)
    , texturePath(cubemapDirectory == nullptr ? SKYBOX_PATH : cubemapDirectory)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    if (cubemapDirectory)
        _reload();
}
void TextureCubeMap::reload() { _reload(); }
void TextureCubeMap::_reload()
{
    SDL_Surface *image;
    for (int i = 0; i < sizeof(FACES) / sizeof(CubeMapParts); i++)
    {
        image = readImage(std::string(texturePath).append(FACES[i].name).c_str());
        setTexture(image, FACES[i].target);
    }
}