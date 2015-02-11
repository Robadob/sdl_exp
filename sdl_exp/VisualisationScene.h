#pragma once

#include <stdio.h>
#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "Camera.h"
#include "Shaders.h"
#include "Axis.h"
#include "Entity.h"

class VisualisationScene
{
public:
	VisualisationScene(Camera* camera = nullptr, Shaders* shaders = nullptr);
	~VisualisationScene();

	void update();
	void render();

private:
	Camera* camera;
	Shaders* shaders;
	Axis* axis;
	Entity* object;
	Entity* mesh256;

	int tick = 0;
	

};

