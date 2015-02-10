#pragma once

#include <stdio.h>
#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "Camera.h"
#include "Axis.h"
#include "Entity.h"

class VisualisationScene
{
public:
	VisualisationScene(Camera* camera = nullptr);
	~VisualisationScene();

	void update();
	void render();

private:
	Camera* camera;
	Axis* axis;
	Entity* object;
	Entity* mesh256;

	int tick = 0;
	

};

