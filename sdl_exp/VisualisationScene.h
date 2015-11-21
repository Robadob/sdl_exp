#pragma once

#include <stdio.h>
#include "gl/glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Camera.h"
#include "Shaders.h"

class VisualisationScene
{
public:
    VisualisationScene(Camera* camera = nullptr);
    ~VisualisationScene();

    void update();
    void reload();
    void render(glm::mat4 projection);
    void generate();

private:
    Camera* camera;

};

