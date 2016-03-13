#ifndef __Texture_h__
#define __Texture_h__

#include "GLcheck.h"

class Texture
{
public:
    Texture();
    ~Texture();

    void createTextureBufferObject(GLuint *tbo, GLuint *texture, GLuint size);
    void deleteTextureBufferObject(GLuint *tbo);
};

#endif //ifndef __Texture_h__