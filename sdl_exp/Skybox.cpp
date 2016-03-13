#include "Skybox.h" 
#include "glm/gtc/type_ptr.hpp"
#include "SDL/SDL_image.h"
#include <stdio.h>
#include <string>
/*
Genric cube with vertices wound such that rendered faces are inwards
*/
float points[] = {
    -10.0f, 10.0f, -10.0f,
    -10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,

    -10.0f, -10.0f, 10.0f,
    -10.0f, -10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, -10.0f,
    -10.0f, 10.0f, 10.0f,
    -10.0f, -10.0f, 10.0f,

    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,

    -10.0f, -10.0f, 10.0f,
    -10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, -10.0f, 10.0f,
    -10.0f, -10.0f, 10.0f,

    -10.0f, 10.0f, -10.0f,
    10.0f, 10.0f, -10.0f,
    10.0f, 10.0f, 10.0f,
    10.0f, 10.0f, 10.0f,
    -10.0f, 10.0f, 10.0f,
    -10.0f, 10.0f, -10.0f,

    -10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f, 10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f, 10.0f,
    10.0f, -10.0f, 10.0f
};
Skybox::Skybox(char* texturePath)
    : shaders("../shaders/skybox.v", "../shaders/skybox.f")
    , texturePath(texturePath)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

   //Init the texture
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    //Load the textures
    this->reloadTextures();
    
    //Pass the textures to openGL
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points, GL_STATIC_DRAW);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
SDL_Surface *Skybox::readTex(const char *texturePath)
{
    std::string imagePath;
    imagePath.assign(texturePath).append(".png");
    SDL_Surface *image = IMG_Load(imagePath.c_str());
    if (!image)
    {
        imagePath.assign(texturePath).append(".bmp");
        image = IMG_Load(imagePath.c_str());
        if (!image)
        {
            printf("Couldn't read tex: %s\n", IMG_GetError());
            exit(1);
        }
    }
    if (image->format->BytesPerPixel != 3 && image->format->BytesPerPixel != 4)
    {
        printf("Textures must be RGB or RGBA with 8 bits per colour.\n");
        exit(1);
    }
    return image;
}
Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texName);
}
void Skybox::reloadTextures()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
    SDL_Surface *image;
    std::string imagePath;
    imagePath.assign(texturePath).append("left");
    image = readTex(imagePath.c_str());
    GLint format = image->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    imagePath.assign(texturePath).append("right");
    image = readTex(imagePath.c_str());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    imagePath.assign(texturePath).append("up");
    image = readTex(imagePath.c_str());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    imagePath.assign(texturePath).append("down");
    image = readTex(imagePath.c_str());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    imagePath.assign(texturePath).append("front");
    image = readTex(imagePath.c_str());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    imagePath.assign(texturePath).append("back");
    image = readTex(imagePath.c_str());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    SDL_FreeSurface(image);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void Skybox::reload()
{
    shaders.reloadShaders(true);
    this->reloadTextures();
}
void Skybox::render(Camera *camera, glm::mat4 projection)
{
    glPushMatrix();
    //Setup shaders
    shaders.useProgram();
    shaders.setUniformMatrix4fv(1, &camera->skyboxView()[0][0]);
    shaders.setUniformMatrix4fv(2, &projection[0][0]);
    shaders.setUniformi(3, 0);

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    //glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
    //glBindSampler(0, linearFiltering);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    glDisableVertexAttribArray(0);

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
    shaders.clearProgram();
}