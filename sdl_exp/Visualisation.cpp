#include "Visualisation.h"
#include "math_helper.h"
#include <math.h>


#define FOV 60.0
#define NEAR_CLIP 0.1
#define FAR_CLIP 100.0
#define DELTA_THETA_PHI 0.05
#define MOUSE_SPEED 0.005

Visualisation::Visualisation(char* windowTitle, int windowWidth, int windowHeight) : quit(false){
	this->windowTitle = windowTitle;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	double radius = 10.0;
	double theta = math_helper::toRadians(45);
	double phi = math_helper::toRadians(45);
	this->camera = Camera(theta, phi, radius);
	this->scene = VisualisationScene(&this->camera);
}

Visualisation::~Visualisation(){
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

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		// Get context
		this->context = SDL_GL_CreateContext(window);

		// Setup gl stuff
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glShadeModel(GL_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		/*glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);*/
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_NORMALIZE);

		// Setup the projection matrix
		this->resizeWindow();
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



void Visualisation::close(){
	SDL_GL_DeleteContext(this->context);
	SDL_DestroyWindow(this->window);
	this->window = NULL;
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

			// Handle continues press keys (movement)
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if (state[SDL_SCANCODE_W]) {
				this->camera.updateThetaPhi(0, -DELTA_THETA_PHI);
			}
			if (state[SDL_SCANCODE_A]) {
				this->camera.updateThetaPhi(-DELTA_THETA_PHI, 0);
			}
			if (state[SDL_SCANCODE_S]) {
				this->camera.updateThetaPhi(0, DELTA_THETA_PHI);
			}
			if (state[SDL_SCANCODE_D]) {
				this->camera.updateThetaPhi(DELTA_THETA_PHI, 0);
			}
			

			// handle each event on the queue
			while (SDL_PollEvent(&e) != 0){
				switch (e.type){
					case SDL_QUIT:
						this->setQuit(true);
						break;
					case SDL_KEYDOWN:
						{
							int x = 0;
							int y = 0;
							SDL_GetMouseState(&x, &y);
							this->handleKeypress(e.key.keysym.sym, x, y);
						}
						break;
					case SDL_MOUSEWHEEL:
						this->camera.updateRadius(-e.wheel.y);
						break;

					case SDL_MOUSEMOTION:
						this->handleMouseMove(e.motion.xrel, e.motion.yrel);
						break;
					case SDL_MOUSEBUTTONDOWN:
						this->toggleMouseMode();
						break;

				}
			}

			// update
			this->scene.update();
			// render
			this->scene.render();
			// update the screen
			SDL_GL_SwapWindow(window);
		}
		SDL_StopTextInput();

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
	if (this->isFullscreen()){
		// Update the window using the stored windowBounds
		SDL_SetWindowBordered(this->window, SDL_TRUE);
		SDL_SetWindowSize(this->window, this->windowedBounds.w, this->windowedBounds.h);
		SDL_SetWindowPosition(this->window, this->windowedBounds.x, this->windowedBounds.y);
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
	}
	this->resizeWindow();
}

void Visualisation::toggleMouseMode(){
	if (SDL_GetRelativeMouseMode()){
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}

void Visualisation::resizeWindow(){
	// Use the sdl drawable size
	int width;
	int height;

	SDL_GL_GetDrawableSize(this->window, &width, &height);

	double fAspect = (double)width / (double)height;
	double fovy = FOV;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double top = tan(math_helper::toRadians(fovy * 0.5)) * NEAR_CLIP;
	double bottom = -top;
	double left = fAspect * bottom;
	double right = fAspect * top;
	glFrustum(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
	//gluPerspective(fovy, fAspect, NEAR_CLIP, FAR_CLIP);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void Visualisation::handleMouseMove(int x, int y){
	if (SDL_GetRelativeMouseMode()){
		this->camera.updateThetaPhi(x * MOUSE_SPEED, y * MOUSE_SPEED);
	}
}

bool Visualisation::isFullscreen(){
	// Use window borders as a toggle to detect fullscreen.
	return SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS;
}