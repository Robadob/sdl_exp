#pragma once

#include <stdio.h>
#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "VisualisationScene.h"
#include "Camera.h"

#undef main


#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

class Visualisation
{
public:
	Visualisation(char* windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT);
	~Visualisation();

	bool init();
	void handleKeypress(SDL_Keycode keycode, int x, int y);
	void close();
	void run(); // @todo - improve

	char* getWindowTitle();
	void setWindowTitle(char* windowTitle);
	
	void setQuit(bool quit);
	void toggleFullScreen();
	void toggleMouseMode();
	void resizeWindow();
	void handleMouseMove(int x, int y);
	bool isFullscreen();


private:
	SDL_Window* window;
	SDL_GLContext context;
	Camera camera;
	VisualisationScene* scene;

	bool isInitialised;
	bool quit;

	char* windowTitle;
	int windowWidth;
	int windowHeight;
	
	SDL_Rect windowedBounds;

};

