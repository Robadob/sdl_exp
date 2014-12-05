#include "Visualisation.h"


Visualisation::Visualisation(char* windowTitle, int windowWidth, int windowHeight)
{
	this->windowTitle = windowTitle;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->quit = false;
	this->isFullscreen = false;
}


Visualisation::~Visualisation()
{
}


bool Visualisation::init(){
	bool result = true;
	
	window = SDL_CreateWindow
		(
		this->windowTitle,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		this->windowWidth,
		this->windowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
		);

	if (window == NULL){
		printf("window failed to init");
		result = false;
	}
	else {
		SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
		SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);

		// Setup renderer
		renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
	}
	return result;
}

void Visualisation::handleKeypress(SDL_Keycode keycode, int x, int y){

	switch (keycode){
	case SDLK_ESCAPE:
		this->setQuit(true);
		break;
	case SDLK_F11:
		this->toggleFullScreen();
		break;
	default:
		// Do nothing?
		break;
	}
}


void Visualisation::update(){
	// Do nothing.
}

void Visualisation::render(){
	//	glClear( GL_COLOR_BUFFER_BIT)

	// Set render color to red ( background will be rendered in this color )
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	// Clear winow
	SDL_RenderClear(renderer);

	// Creat a rect at pos ( 50, 50 ) that's 50 pixels wide and 50 pixels high.
	SDL_Rect r;
	r.x = 50;
	r.y = 50;
	r.w = 50;
	r.h = 50;

	// Set render color to blue ( rect will be rendered in this color )
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

	// Render rect
	SDL_RenderFillRect(renderer, &r);

	// Render the rect to the screen
	SDL_RenderPresent(renderer);
}

void Visualisation::close(){
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
}

void Visualisation::run(){
	if (!this->init()){
		printf("Init fail");
	}
	else {
		SDL_Event e;
		SDL_StartTextInput();
		while (!this->quit){
			// handle each event on the queue
			while (SDL_PollEvent(&e) != 0){
				if (e.type == SDL_QUIT){
					this->setQuit(true);
				}
				else if (e.type == SDL_KEYUP){
					int x = 0;
					int y = 0;
					SDL_GetMouseState(&x, &y);
					handleKeypress(e.key.keysym.sym, x, y);
				}
			}

			// update
			update();
			// render
			render();
			// update the screen
			SDL_GL_SwapWindow(window);
		}
		SDL_StopTextInput();
		SDL_GL_SwapWindow(window);
	}

	this->close();
}

char* Visualisation::getWindowTitle(){
	return this->windowTitle;
}

void Visualisation::setWindowTitle(char* windowTitle){
	this->windowTitle = windowTitle;
}

void Visualisation::setQuit(bool quit){
	this->quit = quit;
}

void Visualisation::toggleFullScreen(){
	if (this->isFullscreen){
		// Update the window using the stored windowBounds
		SDL_SetWindowBordered(this->window, SDL_TRUE);
		SDL_SetWindowSize(this->window, this->windowedBounds.w, this->windowedBounds.h);
		SDL_SetWindowPosition(this->window, this->windowedBounds.x, this->windowedBounds.y);
		// Update somewhat pointles member var
		this->isFullscreen = false;
	}
	else {
		// Store the windowedBounds for later
		SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
		SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);
		// Get the window bounds for the current screen
		int displayIndex = SDL_GetWindowDisplayIndex(this->window);
		SDL_Rect displayBounds;
		SDL_GetDisplayBounds(displayIndex, &displayBounds);
		// Update the window
		SDL_SetWindowBordered(this->window, SDL_FALSE);
		SDL_SetWindowPosition(this->window, displayBounds.x, displayBounds.y);
		SDL_SetWindowSize(this->window, displayBounds.w, displayBounds.h);
		// Update class var, which is kinda pointless.
		this->isFullscreen = true;
	}
}