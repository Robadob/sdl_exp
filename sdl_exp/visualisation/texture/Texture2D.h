#ifndef __Texture2D_h__
#define __Texture2D_h__
#include "Texture.h"

class Texture2D : public Texture
{
public:
    Texture2D(const char *texturePath, char *uniformName = 0);
    virtual ~Texture2D(){}
    void reload() override;

private:
    void _reload();//Used so we don't call a virtual fn from the constructor
    const char *texturePath;
};
#endif //ifndef __Texture2D_h__