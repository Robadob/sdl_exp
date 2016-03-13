#include "Texture.h"
#include <iostream>


Texture::Texture()
{
}


Texture::~Texture()
{
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
