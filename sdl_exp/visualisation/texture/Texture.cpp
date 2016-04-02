#include "Texture.h"

/*
Image extensions to be supported
*/
const char* Texture::IMAGE_EXTS[] = {
    "",
    ".tga",
    ".png",
    ".bmp"
};
/*
@param type The type of texture, e.g. GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
@param texPath This value is only used to block null textures (to save generating unused textures), any value that evaluates to true will suffice
@param uniformName The name of the uniform sampler within the sahder, this defaults to TEXTURE_UNIFORM_NAME from this classes header
*/
Texture::Texture(GLenum type, const char *texPath, char *uniformName)
    : texName(0)
    , texType(type)
    , uniformName(uniformName == 0 ? TEXTURE_UNIFORM_NAME : uniformName)
{
    if (texPath)
        createGLTex();
}
/*
Deletes the GL texture
@note This wraps deleteGLTex()
*/
Texture::~Texture()
{
    deleteGLTex();
}
/*
Creates a GL texture and configures some of it's parameters
@note This does not store an image in the texture
*/
void Texture::createGLTex()
{
    GL_CALL(glGenTextures(1, &texName));
    GL_CALL(glBindTexture(texType, texName));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_WRAP_R, GL_REPEAT));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_BASE_LEVEL, 0));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAX_LEVEL, 0));//Changing this kills textures (why?)
    //GL_CALL(glGenerateMipmap(texType));//Auto generate texture mipmap
}
/*
Deletes the GL texture
*/
void Texture::deleteGLTex()
{
    if (texName)
    {
        GL_CALL(glDeleteTextures(1, &texName));
        texName = 0;        
    }
}
/*
Returns the first image found at the provided path
This method attempts all the suffices stored in Texture::IMAGE_EXTS
@imagePath Path to search for images
*/
SDL_Surface *Texture::findImage(const char *imagePath)
{
    if (!imagePath)
        return 0;
    SDL_Surface *image=0;
    for (int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*);i++)
    {
        image = IMG_Load(std::string(imagePath).append(IMAGE_EXTS[i]).c_str());
        if (image)
        {
            SDL_ClearError();//Clear the img errorS  
            break;          
        }
    }
    return image;
}
/*
Loads a texture from the provided .png or .bmp file
@param texturePath Path to the texture to be loaded
@param printErr True if errors should be printed when an error is detected
@note the SDL_Surface must be freed using SDL_FreeSurface()
*/
SDL_Surface *Texture::readImage(const char *texturePath, bool printErr){
    SDL_Surface *image = findImage(texturePath);
    if (!image)
    {
        if (printErr)
            fprintf(stderr, "%s\n", IMG_GetError());
    }
    else if (image->format->BytesPerPixel != 3 && image->format->BytesPerPixel != 4)
    {
        if (printErr)
            fprintf(stderr, "'%s': Textures must be RGB or RGBA with 8 bits per colour.\n", texturePath);
        SDL_FreeSurface(image);
        return 0;
    }
    else if (image->format->Rshift>image->format->Bshift)
    {   //If the teture is BGR order rathern than RGB order, switch bytes
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
/*
Loads a texture from the provided .png or .bmp file
@param image The image to be stored in the texture
@param target The type of texture to be loaded, the instance variable 'texType' is probably the right value here and is used as a default.
@param dontFreeImage If true the image pointer won't be freed
*/
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
/*
Binds the texture to the passed shader so it will be used at render
@param shaders The shader to bind to
@param uniformName The uniform name within the shader to bind to (defaults to the value of TEXTURE_UNIFORM_NAME)
*/
bool Texture::bindToShader(Shaders *shaders, char *uniformName)
{
    if (!this->texName)
        return false;
    if (uniformName)
        shaders->addTextureUniform(this->texName, uniformName, texType);
    else
        shaders->addTextureUniform(this->texName, this->uniformName, texType);
    return true;
}

//Unused TBO methods, maybe use later if CUDA integration is supported
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