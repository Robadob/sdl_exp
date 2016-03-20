#ifndef __TextureCubeMap_h__
#define __TextureCubeMap_h__
#include "Texture.h"


class TextureCubeMap : public Texture
{
public:
    const char *SKYBOX_PATH = "../textures/skybox/";
    struct CubeMapParts
    {
        CubeMapParts(GLenum target, const char *name)
            : target(target), name(name) {}
        GLenum target;
        const char *name;
    };
    static const CubeMapParts FACES[6];

    TextureCubeMap(const char *texturePath = 0, char *uniformName = 0);
    void reload() override;
private:
    const char *texturePath;
};
#endif