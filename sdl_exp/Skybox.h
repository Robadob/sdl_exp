#pragma once
#include "gl/glew.h"

#include "Camera.h"
#include "Shaders.h"
#include "glm/gtc/type_ptr.hpp"

#define TEXTURE_SIZE 64

class Skybox
{
public:
    /*
    Point the Skybox to a folder containing files named left/right/back/front/up/down.png (bmp is also suitable)
    */
    Skybox(char* texturePath = "../textures/skybox/");
    ~Skybox();
    void render(Camera *camera, glm::mat4 projection);
    void reload();
private:
    void reloadTextures();
    char* texturePath;
    SDL_Surface *readTex(const char *texturePath);
    Shaders shaders;
    GLuint texName;
    GLuint vbo;
    GLuint vao;

};