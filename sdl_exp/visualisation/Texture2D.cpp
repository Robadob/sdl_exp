#include "Texture2D.h"


Texture2D::Texture2D(const char *texturePath)
    :Texture(GL_TEXTURE_2D)
{
    SDL_Surface *image = readImage(texturePath);
    setTexture(image);
}
