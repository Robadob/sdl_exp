#include "Texture2D.h"


Texture2D::Texture2D(const char *texturePath, char *uniformName)
    : Texture(GL_TEXTURE_2D, texturePath, uniformName)
    , texturePath(texturePath)
{
    if (texturePath)
        _reload();
}
void Texture2D::reload() { _reload(); }
void Texture2D::_reload()
{
    setTexture(readImage(texturePath));
}
