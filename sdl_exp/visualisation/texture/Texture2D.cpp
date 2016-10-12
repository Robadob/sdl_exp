#include "Texture2D.h"

/*
Loads the 2D texture
@param texturePath The path to the image to be used as the texture
@param uniformName Name of the uniform to be used, defaults to the value of Texture::TEXTURE_UNIFORM_NAME
*/
Texture2D::Texture2D(const char *texturePath, char *uniformName)
    : Texture(GL_TEXTURE_2D, texturePath, uniformName)
    , texturePath(texturePath)
    , dimensions(0)
{
    _reload();
}
/*
Loads the provided 2D texture into texture memory
This paramters of this function wrap setTexture()
@param uniformName Name of the uniform to be used, defaults to the value of Texture::TEXTURE_UNIFORM_NAME
@see setTexture(...)
*/
Texture2D::Texture2D(char *uniformName, void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type)
    : Texture(GL_TEXTURE_2D, "", uniformName)
    , texturePath(nullptr)
{
    setTexture(image, imageSize, width, height, internalFormat, format, type);
}
/*
Loads a texture from the provided pointer
@param image The pointer to the texture data
@param imageSize Size of the tex pointed to by image
@param width Width of the image
@param height Height of the image
@param internalFormat See GL docs for glTexImage2D
@param format See GL docs for glTexImage2D
@param type See GL docs for glTexImage2D
@note This function will not free the memory, you should do that yourself
@see https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
*/
void Texture2D::setTexture(void *image, size_t imageSize, unsigned int width, unsigned int height, GLint internalFormat, GLenum format, GLenum type)
{
    if (image == nullptr)
        return;

    GL_CALL(glBindTexture(texType, texName));
    if (width%4!=0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    GL_CALL(glTexImage2D(texType, 0, internalFormat, width, height, 0, format, type, image));
    GL_CALL(glBindTexture(texType, 0));
}
/*
Loads the 2D texture
@overrides Texture::reload()
*/
void Texture2D::reload() {
    _reload();
}
/*
Loads the 2D texture
@note this method is required, so that the constructor doesn't call a virtual function
*/
void Texture2D::_reload()
{
    if (texturePath)
    {
        SDL_Surface *img = readImage(texturePath);
        if (!img)
            return;
        dimensions = glm::uvec2((unsigned int)img->w, (unsigned int)img->h);
        Texture::setTexture(img);
    }
}
