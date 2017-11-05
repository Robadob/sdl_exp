#include "Sprite2D.h"
#include "shader/Shaders.h"


Sprite2D::Sprite2D(const char *imagePath, std::shared_ptr<Shaders> shader, unsigned int _width, unsigned int _height)
	: Overlay(shader == nullptr ? std::make_shared<Shaders>(Stock::Shaders::SPRITE2D):shader)
    , tex(Texture2D::load(imagePath))
{
    unsigned int width = _width == 0 ? tex->getWidth() : _width;
    unsigned int height = _height == 0 ? (_width == 0 ? tex->getHeight() : (tex->getHeight()*width / tex->getWidth())) : _height;
    setDimensions(width, height);
    getShaders()->addTexture("_texture", tex);
}
Sprite2D::Sprite2D(GLuint texName, unsigned int width, unsigned int height, std::shared_ptr<Shaders> shader)
	: Overlay(shader == nullptr ? std::make_shared<Shaders>(Stock::Shaders::SPRITE2D) : shader)
    , tex(nullptr)
{
    setDimensions(width, height);
    getShaders()->addTextureUniform(texName, "_texture", GL_TEXTURE_2D);
}