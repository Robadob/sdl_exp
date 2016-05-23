#include "Sprite2D.h"

/*
Creates a new 2D Sprite overlay from the given image
@param imagePath Path to the image to be used
@param width The width of the overlay, if 0 is passed the image width will be used
@param height The height of the overlay, if 0 is passed, the image height will be used unless width was specified, in which case the image will be scaled, preserving aspect ratio
*/
Sprite2D::Sprite2D(char *imagePath, unsigned int _width, unsigned int _height) 
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SPRITE2D))
    , tex(imagePath)
{
    unsigned int width = _width == 0 ? tex.getWidth() : _width;
    unsigned int height = _height == 0 ? (_width == 0 ? tex.getHeight() : (tex.getHeight()*width / tex.getWidth())) : _height;
    setDimensions(width, height);
    tex.bindToShader(getShaders().get());
}