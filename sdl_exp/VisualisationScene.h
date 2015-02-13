#pragma once

#include <stdio.h>
#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "Camera.h"
#include "Shaders.h"
#include "Axis.h"
#include "Entity.h"
#include "Texture.h"

class VisualisationScene
{
public:
	VisualisationScene(Camera* camera = nullptr, Shaders* vechShaders = nullptr, Shaders* envShaders = nullptr);
	~VisualisationScene();

	void update();
	void render();

private:
	Camera* camera;
	Shaders* vechShaders;
	Shaders* envShaders;

	Axis* axis;
	Entity* object;
	Entity* mesh256;
	// Temp stuff to move elsewhere at a later date
	int agentCount = 0;
	Texture* agent_texture; 
	GLuint agent_position_data_tbo;
	GLuint agent_position_data_tex;
	int environmentCount = 0;
	Texture* environment_texture;
	GLuint environment_position_data_tbo;
	GLuint environment_position_data_tex;

	int tick = 0;
	

};

