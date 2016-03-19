#include "Texture.h"

#include <string>

Texture::Texture(const char *texturePath)
    :texturePath(texturePath)
{
    //init texture
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName); 
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glGenerateMipmap(GL_TEXTURE_2D);//Auto generate texture mipmaps
    SDL_Surface *image = readTex(texturePath);
    printf("\ntex: (%i, %i, %i)\n", image->format->Rshift, image->format->Gshift, image->format->Bshift);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
    //glPixelStorei(GL_PACK_ALIGNMENT, 1);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint format = image->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
}


Texture::~Texture()
{
    glDeleteTextures(1, &texName);
}

/*
Loads a texture from the provided .png or .bmp file
@param texturePath Path to the texture to be loaded
@note the SDL_Surface must be freed using SDL_FreeSurface()
*/
SDL_Surface *Texture::readTex(const char *texturePath){
    SDL_Surface *image = IMG_Load(texturePath);
    if (!image)
    {
        fprintf(stderr, "Couldn't read texure file '%s': %s\n", texturePath, IMG_GetError());
    }
    else if (image->format->BytesPerPixel != 3 && image->format->BytesPerPixel != 4)
    {
        fprintf(stderr, "'%s': Textures must be RGB or RGBA with 8 bits per colour.\n", texturePath);
    }
    return image;
}

void Texture::createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size){
    glGenTextures(1, texture);
    glGenBuffers(1, tbo);
    
    glBindBuffer(GL_TEXTURE_BUFFER, *tbo);
    glBufferData(GL_TEXTURE_BUFFER, size, 0, GL_STATIC_DRAW);

    glBindTexture(GL_TEXTURE_BUFFER, *texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, *tbo);

    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    GL_CHECK();
}

void Texture::deleteTextureBufferObject(GLuint *tbo){
    glDeleteBuffers(1, tbo);
}

void Texture::bindToShader(Shaders *shaders, char *uniformName)
{
    if (uniformName)
        shaders->addTextureUniform(texName, uniformName, GL_TEXTURE_2D);
    else
        shaders->addTextureUniform(texName, TEXTURE_UNIFORM_NAME, GL_TEXTURE_2D);
}