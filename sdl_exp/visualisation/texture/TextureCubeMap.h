#ifndef __TextureCubeMap_h__
#define __TextureCubeMap_h__
#include "Texture.h"


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
    static const CubeMapParts FACES[6];

    TextureCubeMap(const char *texturePath = 0, char *uniformName = 0);
    virtual ~TextureCubeMap(){}
    void reload() override;
private:
    void _reload();//Used so we don't call a virtual fn from the constructor
    const char *texturePath;
};
#endif