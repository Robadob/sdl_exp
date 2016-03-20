#include "Texture.h"

Texture::Texture(GLenum type)
: texName(0)
, texType(type)
{
    createGLTex();
}
Texture::~Texture()
{
    deleteGLTex();
}
 

void Texture::createGLTex()
{
    //init texture
    GL_CALL(glGenTextures(1, &texName));
    GL_CALL(glBindTexture(texType, texName));
    //glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_BASE_LEVEL, 0));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAX_LEVEL, 0));
    GL_CALL(glGenerateMipmap(texType));//Auto generate texture mipmap

}

void Texture::deleteGLTex()
{
    if (texName)
        GL_CALL(glDeleteTextures(1, &texName));
}

/*
Loads a texture from the provided .png or .bmp file
@param texturePath Path to the texture to be loaded
@note the SDL_Surface must be freed using SDL_FreeSurface()
*/
SDL_Surface *Texture::readImage(const char *texturePath, bool printErr){
    SDL_Surface *image = IMG_Load(texturePath);
    if (!image)
    {
        if (printErr)
            fprintf(stderr, "Couldn't read texure file '%s': %s\n", texturePath, IMG_GetError());
    }
    else if (image->format->BytesPerPixel != 3 && image->format->BytesPerPixel != 4)
    {
        if (printErr)
            fprintf(stderr, "'%s': Textures must be RGB or RGBA with 8 bits per colour.\n", texturePath);
        SDL_FreeSurface(image);
        return 0;
    }
    //If the teture is BGR order rathern than RGB order, switch bytes
    if (image->format->Rshift>image->format->Bshift)
    {
        SDL_PixelFormat desiredFormat;
        memcpy(&desiredFormat, image->format, sizeof(SDL_PixelFormat));
        desiredFormat.Bloss = image->format->Rloss;
        desiredFormat.Bmask = image->format->Rmask;
        desiredFormat.Bshift = image->format->Rshift;
        desiredFormat.Rloss = image->format->Bloss;
        desiredFormat.Rmask = image->format->Bmask;
        desiredFormat.Rshift = image->format->Bshift;
        if (image->format->BytesPerPixel == 3)
            desiredFormat.format = SDL_PIXELFORMAT_RGB888;
        else
            desiredFormat.format = SDL_PIXELFORMAT_RGBA8888;
        SDL_Surface* old = image;
        image = SDL_ConvertSurface(old, &desiredFormat, 0);
        SDL_FreeSurface(old);
    }
    return image;
}

void Texture::setTexture(SDL_Surface *image, GLuint target, bool dontFreeImage)
{
    if (image == 0)
        return;
    if (target == 0)
        target = texType;

    GLint internalFormat = image->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA;

    GL_CALL(glBindTexture(texType, texName));
    GL_CALL(glTexImage2D(target, 0, internalFormat, image->w, image->h, 0, internalFormat, GL_UNSIGNED_BYTE, image->pixels));
    GL_CALL(glBindTexture(texType, 0));

    if (!dontFreeImage)
        SDL_FreeSurface(image);
}
//void Texture::createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size){
//    glGenTextures(1, texture);
//    glGenBuffers(1, tbo);
//    
//    glBindBuffer(GL_TEXTURE_BUFFER, *tbo);
//    glBufferData(GL_TEXTURE_BUFFER, size, 0, GL_STATIC_DRAW);
//
//    glBindTexture(GL_TEXTURE_BUFFER, *texture);
//    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, *tbo);
//
//    glBindBuffer(GL_TEXTURE_BUFFER, 0);
//    glBindTexture(GL_TEXTURE_BUFFER, 0);
//
//    GL_CHECK();
//}
//
//void Texture::deleteTextureBufferObject(GLuint *tbo){
//    glDeleteBuffers(1, tbo);
//}

void Texture::bindToShader(Shaders *shaders, char *uniformName)
{
    if (uniformName&&texName)
        shaders->addTextureUniform(texName, uniformName, GL_TEXTURE_2D);
    else
        shaders->addTextureUniform(texName, TEXTURE_UNIFORM_NAME, GL_TEXTURE_2D);
}