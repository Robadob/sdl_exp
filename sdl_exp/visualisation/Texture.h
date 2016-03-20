#ifndef __Texture_h__
#define __Texture_h__

#include "GLcheck.h"
#include "Shaders.h"

#include <SDL/SDL_image.h>

class Texture
{
    static const char *IMAGE_EXTS[4];
    char *TEXTURE_UNIFORM_NAME = "_texture";//cant make const because of where it gets passed
public:
    virtual void reload() = 0;
    //void createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size);
    //void deleteTextureBufferObject(GLuint *tbo); 
    
    bool bindToShader(Shaders *s, char *uniformName = 0);
protected:
    Texture(GLenum type, char *uniformName = 0);
    ~Texture();
    static SDL_Surface *readImage(const char *texturePath, bool printErr = true);
    void setTexture(SDL_Surface *image, GLuint target = 0, bool dontFreeImage = false);
    GLuint texName;
private:
    static SDL_Surface *Texture::findImage(const char *imagePath);
    const GLenum texType;
    char *uniformName;
    void createGLTex();
    void deleteGLTex();
};

#endif //ifndef __Texture_h__