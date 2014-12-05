#pragma once

#include <stdio.h>
#include "SDL/SDL.h"
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
	void update();
	void render();
	void close();
	void run(); // @todo - improve

	char* getWindowTitle();
	void setWindowTitle(char* windowTitle);
	
	void setQuit(bool quit);
	void toggleFullScreen();


private:
	SDL_Window* window;
	SDL_Renderer* renderer;

	bool quit = false;

	char* windowTitle;
	int windowWidth;
	int windowHeight;
	
	bool isFullscreen;

	SDL_Rect windowedBounds;
};

