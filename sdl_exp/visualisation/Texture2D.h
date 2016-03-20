#ifndef __Texture2D_h__
#define __Texture2D_h__
#include "Texture.h"


class Texture2D : public Texture
{
public:
    Texture2D(const char *texturePath, char *uniformName = 0);
    void reload() override;

private:
    const char *texturePath;
};
#endif //ifndef __Texture2D_h__