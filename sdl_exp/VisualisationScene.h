#pragma once

#include <stdio.h>
#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "Camera.h"

class VisualisationScene
{
public:
	VisualisationScene(Camera* camera = nullptr);
	~VisualisationScene();

	void update();
	void render();

private:
	Camera* camera;


};

