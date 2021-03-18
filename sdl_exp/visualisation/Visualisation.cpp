#include "Visualisation.h"

#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.inl>

#include "util/GLcheck.h"
#include "interface/Scene.h"

#include "Text.h"

#define FOVY 60.0f
#define NEAR_CLIP 0.1f
#define FAR_CLIP 10000.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f
#define SHIFT_MULTIPLIER 5.0f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_MOVE 0.05f
#define DELTA_STRAFE 0.05f
#define DELTA_ASCEND 0.05f
#define DELTA_ROLL 0.01f
#define ONE_SECOND_MS 1000
#define VSYNC 1

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

Visualisation::Visualisation(char *windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT)
    : t(nullptr)
    , hud(std::make_shared<HUD>(windowWidth, windowHeight))
    , camera(std::make_shared<NoClipCamera>(glm::vec3(100)))
    , scene(nullptr)
    , isInitialised(false)
	, continueRender(false)
    , msaaState(true)
    , windowTitle(windowTitle)
    , windowDims(windowWidth, windowHeight)
	, fpsDisplay(nullptr)
{
    this->isInitialised = this->init();

    fpsDisplay = std::make_shared<Text>("", 10, glm::vec3(1.0f), Stock::Font::ARIAL);
    fpsDisplay->setUseAA(false);
    hud->add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::West, glm::ivec2(0), INT_MAX);
    helpText = std::make_shared<Text>("Controls\nW,S: Move Forwards/Backwards\nA,D: Strafe\nQ,E: Roll\nF1:  Toggle Show Controls\nF5:  Reload Resources/Shaders\nF8:  Toggle Show FPS\nF9:  Toggle Show Skybox\nF10: Toggle MSAA\nF11: Toggle Fullscreen\nESC: Quit", 20, glm::vec3(1.0f), Stock::Font::LUCIDIA_CONSOLE);
    helpText->setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));
    helpText->setVisible(false);
    hud->add(helpText, HUD::AnchorV::Center, HUD::AnchorH::Center, glm::ivec2(0), INT_MAX);
}
Visualisation::~Visualisation()
{
	this->close();
}
bool Visualisation::init(){
    SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    // Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    //Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    this->window = SDL_CreateWindow
        (
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowDims.x,
        this->windowDims.y,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
        );

    if (!this->window){
        printf("Window failed to init.\n");
    }
    else {
        SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
        SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);

        // Get context
        this->context = SDL_GL_CreateContext(window);

        //Enable VSync
        int swapIntervalResult = SDL_GL_SetSwapInterval(VSYNC);
        if (swapIntervalResult == -1){
            printf("Swap Interval Failed: %s\n", SDL_GetError());
        }
        
        GLEW_INIT();
        
        // Setup gl stuff
		GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glCullFace(GL_BACK));
        GL_CALL(glEnable(GL_CULL_FACE));
        GL_CALL(glShadeModel(GL_SMOOTH));
        GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        GL_CALL(glBlendEquation(GL_FUNC_ADD));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        BackBuffer::setClear(true, glm::vec3(0));//Clear to black
        setMSAA(this->msaaState);

        // Setup the projection matrix
        this->resizeWindow();
        GL_CHECK();
        return true;
    }
    return false;
}
std::shared_ptr<Scene> Visualisation::setScene(std::unique_ptr<Scene> scene)
{
	std::shared_ptr<Scene> oldScene = this->scene;
	this->scene = std::shared_ptr<Scene>(scene.release());
    return oldScene;
}
void Visualisation::handleMouseMove(int x, int y){
    if (SDL_GetRelativeMouseMode()){
        this->camera->turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
    }
}
void Visualisation::handleKeypress(SDL_Keycode keycode, int x, int y){
    //Pass key events to the scene and skip handling if false is returned 
    if (scene&&!scene->_keypress(keycode, x, y))
        return;
    switch (keycode){
    case SDLK_ESCAPE:
        this->quit();
        break;
    case SDLK_F1:
        this->helpText->setVisible(!this->helpText->getVisible());
        break;
    case SDLK_F11:
        this->toggleFullScreen();
        break;
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F8:
        this->fpsDisplay->setVisible(!this->fpsDisplay->getVisible());
        break;
    case SDLK_F5:
        if (this->scene)
			this->scene->_reload();
		this->hud->reload();
        break;
    default:
        // Do nothing?
        break;
    }
}
void Visualisation::close(){
	killThread();
	assert(this->window);//There should always be a window, it might just be hidden
	SDL_GL_MakeCurrent(this->window, this->context);
    //Delete objects before we delete the GL context!
    fpsDisplay.reset();
	helpText.reset();
    this->hud->clear();
    if (this->scene)
    {
        this->scene.reset();
	}
	SDL_DestroyWindow(this->window);
	this->window = nullptr;
    SDL_GL_DeleteContext(this->context);
    SDL_Quit();
}
void Visualisation::render()
{
    //Static fn var for tracking the time to send to scene->update()
    static unsigned int updateTime = 0;
	unsigned int t_updateTime = SDL_GetTicks();
	//If the program runs for over ~49 days, the return value of SDL_GetTicks() will wrap
	unsigned int frameTime = t_updateTime < updateTime ? (t_updateTime + (UINT_MAX - updateTime)) : t_updateTime - updateTime;
	updateTime = t_updateTime;
    SDL_Event e;
    // Handle continuous key presses (movement)
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float turboMultiplier = state[SDL_SCANCODE_LSHIFT] ? SHIFT_MULTIPLIER : 1.0f;
	turboMultiplier *= frameTime;
    if (state[SDL_SCANCODE_W]) {
        this->camera->move(DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_A]) {
        this->camera->strafe(-DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_S]) {
        this->camera->move(-DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_D]) {
        this->camera->strafe(DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_Q]) {
        this->camera->roll(-DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_E]) {
        this->camera->roll(DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        this->camera->ascend(DELTA_ASCEND*turboMultiplier);
    }
    if (state[SDL_SCANCODE_LCTRL]) {
        this->camera->ascend(-DELTA_ASCEND*turboMultiplier);
    }
    const int mouse_magnitude = 2;
    if (state[SDL_SCANCODE_I]) {
        this->handleMouseMove(0, -mouse_magnitude);
    }
    if (state[SDL_SCANCODE_K]) {
        this->handleMouseMove(0, mouse_magnitude);
    }
    if (state[SDL_SCANCODE_J]) {
        this->handleMouseMove(-mouse_magnitude, 0);
    }
    if (state[SDL_SCANCODE_L]) {
        this->handleMouseMove(mouse_magnitude, 0);
    }

    // handle each event on the queue
    while (SDL_PollEvent(&e) != 0){
        switch (e.type){
        case SDL_QUIT:
            this->quit();
            break;
		//case SDL_WINDOWEVENT:
		//	if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		//		resizeWindow();
		//	break;
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
           // this->handleMouseMove(e.motion.xrel, e.motion.yrel);//Relative mouse mode doesn't work over remote desktop
            break;
        case SDL_MOUSEBUTTONDOWN:
            this->toggleMouseMode();
            break;

        }
    }
    // update
	this->scene->_update(frameTime);
    // render
    BackBuffer::useStatic();
    this->scene->_render();
    GL_CALL(glViewport(0, 0, windowDims.x, windowDims.y));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	this->hud->render();

    GL_CHECK();

    // update the screen
    SDL_GL_SwapWindow(window);
}
void Visualisation::runAsync()
{
	if (!continueRender)
	{
		if (t)
		{//Async window was closed via cross, so thread still exists
			killThread();
		}
		else
		{
			//Curiously, if we destroy a host-thread window from a different thread, the glContext breaks
			//However the inverse is not true
			SDL_GL_MakeCurrent(this->window, NULL);
			SDL_DestroyWindow(this->window);
		}
		this->window = nullptr;
		this->t = new std::thread(&Visualisation::_run, this);
	}
	else
	{
		printf("Already running! Call quit() to close it first!\n");
	}
}
void Visualisation::run()
{
	if (!continueRender)
	{
		//Incase Async window was closed via cross and thread still exists
		killThread();
		_run();
	}
	else
	{
		printf("Already running! Call quit() to close it first!\n");
	}
}
void Visualisation::_run()
{
    if (!this->isInitialised){
        printf("Visulisation not initialised yet.\n");
    }
    else if (!this->scene){
        printf("Scene not yet set.\n");
    }
	else {
		//Recreate window in current thread (else IO fails)
		if (this->window)
		{
			SDL_GL_MakeCurrent(this->window, NULL);
			SDL_DestroyWindow(this->window);
		}
		this->window = SDL_CreateWindow
		(
			this->windowTitle,
			this->windowedBounds.x,
			this->windowedBounds.y,
			this->windowedBounds.w,
			this->windowedBounds.h,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
		);
		if (!this->window){
			printf("Window failed to init.\n");
		}
		else {
			SDL_GL_MakeCurrent(this->window, this->context);
			this->resizeWindow();
			GL_CHECK();
			SDL_StartTextInput();
			this->continueRender = true;
			while (this->continueRender){
				// Update the fps in the window title
				this->updateFPS();

				this->render();
			}
			SDL_StopTextInput();
			//Release mouse lock
			if (SDL_GetRelativeMouseMode()){
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
			//Hide window
			SDL_HideWindow(window);
		}
    }
}
void Visualisation::setMSAA(bool state){
    this->msaaState = state;
    if (this->msaaState)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}
const char *Visualisation::getWindowTitle() const{
    return this->windowTitle;
}
void Visualisation::setWindowTitle(const char *windowTitle){
    this->windowTitle = windowTitle; 
    SDL_SetWindowTitle(this->window, this->windowTitle);
}
void Visualisation::quit(){
	this->continueRender = false;
	if (this->t && std::this_thread::get_id()!= this->t->get_id())
	{
		killThread();
	}
}
void Visualisation::killThread()
{
	if (this->t && std::this_thread::get_id() != this->t->get_id())
	{
		this->continueRender = false;
		//Wait for thread to exit
		this->t->join();
		delete this->t;
		this->t = nullptr;
		//Recreate hidden window in current thread, so context is stable
		SDL_GL_MakeCurrent(this->window, NULL);
		SDL_DestroyWindow(this->window);
		this->window = SDL_CreateWindow
			(
			this->windowTitle,
			this->windowedBounds.x,
			this->windowedBounds.y,
			this->windowedBounds.w,
			this->windowedBounds.h,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
			);
		SDL_GL_MakeCurrent(this->window, this->context);
	}
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
    {
        glm::ivec2 tDims;
        SDL_GL_GetDrawableSize(this->window, &tDims.x, &tDims.y);
        this->windowDims = tDims;
    }
    // Get the view frustum using GLM. Alternatively glm::perspective could be used.
    this->projMat = glm::perspectiveFov<float>(glm::radians(FOVY), (float)this->windowDims.x, (float)this->windowDims.y, NEAR_CLIP, FAR_CLIP);
    // Notify other elements
    this->hud->resizeWindow(this->windowDims);
    if (this->scene)
        this->scene->_resize(this->windowDims);
    resizeBackBuffer(this->windowDims);
}
bool Visualisation::isFullscreen() const{
    // Use window borders as a toggle to detect fullscreen.
    return (SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS;
}
void Visualisation::updateFPS(){
    // Update the current time
    this->currentTime = SDL_GetTicks();
    // Update frame counter
    this->frameCount += 1;
    // If it's been more than a second, do something.
    if (this->currentTime > this->previousTime + ONE_SECOND_MS){
        // Calculate average fps.
        double fps = this->frameCount / double(this->currentTime - this->previousTime) * ONE_SECOND_MS;
        //// Update the title to include FPS at the end.
        //std::ostringstream newTitle;
        //newTitle << this->windowTitle << " (" << std::to_string(static_cast<int>(std::ceil(fps))) << " fps)";
        //SDL_SetWindowTitle(this->window, newTitle.str().c_str());
        //Update the FPS string
        this->fpsDisplay->setString("%.3f fps", fps);

        // reset values;
        this->previousTime = this->currentTime;
        this->frameCount = 0;
    }
}
std::shared_ptr<const Camera> Visualisation::getCamera() const
{
    return this->camera;
}
std::shared_ptr<NoClipCamera> Visualisation::Camera()
{
    return this->camera;
}
std::weak_ptr<Scene> Visualisation::getScene() const{
    return this->scene;
}
const glm::mat4 *Visualisation::getProjectionMatPtr() const{
    return &this->projMat;
}
glm::mat4 Visualisation::getProjectionMat() const{
    return this->projMat;
}
std::weak_ptr<HUD> Visualisation::getHUD(){
	return hud;
}
