#include "Visualisation.h"
#include "math_helper.h"
#include <math.h>
#include <string>
#include <sstream>



#define FOVY 60.0f
#define NEAR_CLIP 0.1f
#define FAR_CLIP 500.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_MOVE 0.1f
#define DELTA_STRAFE 0.1f
#define DELTA_ASCEND 0.1f
#define ONE_SECOND_MS 1000

Visualisation::Visualisation(char* windowTitle, int windowWidth, int windowHeight) : isInitialised(false), quit(false){
	this->windowTitle = windowTitle;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	float theta = static_cast<float>(math_helper::toRadians(135));
    float phi = static_cast<float>(math_helper::toRadians(-35));
    this->camera = Camera(glm::vec3(10),theta, phi);

	this->isInitialised = this->init();
}

Visualisation::~Visualisation(){
	delete this->scene;
	delete this->vechShaders;
	delete this->envShaders;
}


bool Visualisation::init(){
	bool result = true;

	SDL_Init(SDL_INIT_VIDEO);

	this->window = SDL_CreateWindow
		(
		this->windowTitle,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		this->windowWidth,
		this->windowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
		);

	if (this->window == NULL){
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
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);


		
		// Get context
		this->context = SDL_GL_CreateContext(window);

		// Init glew.
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			exit(1);
		}

		// Shader stuff?
		
		this->envShaders = new Shaders("glsl/main.vert", "glsl/main.frag");
		this->vechShaders= new Shaders("glsl/main.vert", "glsl/main.frag");
		//this->vechShaders = new Shaders("glsl/vech.vert", "glsl/vech.frag");

		// Create the scene - need to be done after glew is init
		this->scene = new VisualisationScene(&this->camera, this->vechShaders, this->envShaders);
		

		// Setup gl stuff
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glShadeModel(GL_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		//glEnable(GL_COLOR_MATERIAL);
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
	case SDLK_F5:
		this->vechShaders->reloadShaders();
		this->envShaders->reloadShaders();
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
	if (!this->isInitialised){
		printf("Visulisation not initialised yet.");
	}
	else {
		SDL_Event e;
		SDL_StartTextInput();
		while (!this->quit){
			// Update the fps
			this->updateFPS();

			// Handle continues press keys (movement)
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if (state[SDL_SCANCODE_W]) {
				this->camera.move(-DELTA_MOVE);
			}
			if (state[SDL_SCANCODE_A]) {
				this->camera.strafe(-DELTA_STRAFE);
			}
			if (state[SDL_SCANCODE_S]) {
				this->camera.move(DELTA_MOVE);
			}
			if (state[SDL_SCANCODE_D]) {
				this->camera.strafe(DELTA_STRAFE);
			}
			if (state[SDL_SCANCODE_SPACE]) {
				this->camera.ascend(DELTA_ASCEND);
			}
			if (state[SDL_SCANCODE_LCTRL]) {
				this->camera.ascend(-DELTA_ASCEND);
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
					//case SDL_MOUSEWHEEL:
						
						//break;

					case SDL_MOUSEMOTION:
						this->handleMouseMove(e.motion.xrel, e.motion.yrel);
						break;
					case SDL_MOUSEBUTTONDOWN:
						this->toggleMouseMode();
						break;

				}
			}

			// update
			this->scene->update();
			// render
			this->scene->render(this->frustum);
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

    float fAspect = static_cast<float>(width) / static_cast<float>(height);
	double fovy = FOVY;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    float top = static_cast<float>(tan(math_helper::toRadians(fovy * 0.5)) * NEAR_CLIP);
    float bottom = -top;
    float left = fAspect * bottom;
    float right = fAspect * top;
	//glFrustum(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
	this->frustum = glm::frustum<float>(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
	//gluPerspective(fovy, fAspect, NEAR_CLIP, FAR_CLIP);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

}

void Visualisation::handleMouseMove(int x, int y){
	if (SDL_GetRelativeMouseMode()){
        this->camera.turn(-x * MOUSE_SPEED, -y * MOUSE_SPEED);
	}
}

bool Visualisation::isFullscreen(){
	// Use window borders as a toggle to detect fullscreen.
    return (SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS;
}

// Super simple fps counter imoplementation
void Visualisation::updateFPS(){
	// Update the current time
	this->currentTime = SDL_GetTicks();
	// Update frame counter
	this->frameCount += 1;
	// If it's been more than a second, do something.
	if (this->currentTime > this->previousTime + ONE_SECOND_MS){
		// Calculate average fps.
		double fps = this->frameCount / double(this->currentTime - this->previousTime) * ONE_SECOND_MS;
		// Update the title to include FPS at the end.
		std::ostringstream newTitle;
		newTitle << this->windowTitle << " (" << std::to_string(fps) << " fps)";
		SDL_SetWindowTitle(this->window, newTitle.str().c_str());

		// reset values;
		this->previousTime = this->currentTime;
		this->frameCount = 0;
	}
}