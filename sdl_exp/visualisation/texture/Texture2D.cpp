#include "Texture2D.h"

/*
Loads the 2D texture
@param texturePath The path to the image to be used as the texture
@param uniformName Name of the uniform to be used, defaults to the value of Texture::TEXTURE_UNIFORM_NAME
*/
Texture2D::Texture2D(const char *texturePath, char *uniformName)
    : Texture(GL_TEXTURE_2D, texturePath, uniformName)
    , texturePath(texturePath)
{
    if (texturePath)
        _reload();
}
/*
Loads the 2D texture
@overrides Texture::reload()
*/
void Texture2D::reload() { _reload(); }
/*
Loads the 2D texture
@note this method is required, so that the constructor doesn't call a virtual function
*/
void Texture2D::_reload()
{
    setTexture(readImage(texturePath));
}
