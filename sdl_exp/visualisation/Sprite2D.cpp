#include "Sprite2D.h"


Sprite2D::Sprite2D(char *imagePath, unsigned int _width, unsigned int _height) 
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SPRITE2D))
    , tex(std::make_shared<Texture2D>(imagePath))
{
    unsigned int width = _width == 0 ? tex->getWidth() : _width;
    unsigned int height = _height == 0 ? (_width == 0 ? tex->getHeight() : (tex->getHeight()*width / tex->getWidth())) : _height;
    setDimensions(width, height);
    tex->bindToShader(getShaders().get());
}
Sprite2D::Sprite2D(GLuint texName, unsigned int width, unsigned int height)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SPRITE2D))
    , tex(nullptr)
{
    setDimensions(width, height);
    getShaders()->addTextureUniform(texName, "_texture", GL_TEXTURE_2D);
}