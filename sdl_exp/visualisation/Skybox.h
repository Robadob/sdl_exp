#ifndef __Skybox_h__
#define __Skybox_h__

#include "Camera.h"
#include "Shaders.h"

#include <SDL/SDL.h>

#define TEXTURE_SIZE 64

class Skybox
{
public:
    Skybox(const glm::mat4 *modelViewMat, const glm::mat4 *projectionMat, char *texturePath = "../textures/skybox/");
    ~Skybox();
    void render(Camera *camera, glm::mat4 projection);
    void reload();
private:
    void reloadTextures();
    char *texturePath;
    SDL_Surface *readTex(const char *texturePath);
    Shaders shaders;
    GLuint texName;
    GLuint vbo;
    GLuint vao;

};
#endif //ifndef __Skybox_h__