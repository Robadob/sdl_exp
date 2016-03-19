#ifndef __Texture_h__
#define __Texture_h__

#include "GLcheck.h"
#include "Shaders.h"

#include <SDL/SDL_image.h>

class Texture
{
    char *TEXTURE_UNIFORM_NAME = "_texture";//cant make const because of where it gets passed
public:
    Texture(const char *texturePath);
    ~Texture();

    void createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size);
    void deleteTextureBufferObject(GLuint *tbo); 
    
    void bindToShader(Shaders *s, char *uniformName=0);
private:
    SDL_Surface *Texture::readTex(const char *texturePath);
    const char *texturePath;
    GLuint texName;
};

#endif //ifndef __Texture_h__