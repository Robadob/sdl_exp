#include "Texture2D.h"


Texture2D::Texture2D(const char *texturePath, char *uniformName)
    : Texture(GL_TEXTURE_2D, uniformName)
    , texturePath(texturePath)
{
    reload();
}
void Texture2D::reload()
{
    setTexture(readImage(texturePath));
}
