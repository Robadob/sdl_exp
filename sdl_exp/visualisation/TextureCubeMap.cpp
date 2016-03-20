#include "TextureCubeMap.h"

const TextureCubeMap::CubeMapParts TextureCubeMap::FACES[] = {
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "left"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "right"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "up"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "down"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "front"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "bac")
};

TextureCubeMap::TextureCubeMap(const char *cubemapDirectory, char *uniformName)
    :Texture(GL_TEXTURE_CUBE_MAP, uniformName)
    , texturePath(cubemapDirectory == 0 ? SKYBOX_PATH : cubemapDirectory)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    reload();
}
void TextureCubeMap::reload()
{
    SDL_Surface *image;
    for (int i = 0; i < sizeof(FACES) / sizeof(CubeMapParts); i++)
    {
        image = readImage(std::string(texturePath).append(FACES[i].name).c_str());
        setTexture(image, FACES[i].target);
    }
}