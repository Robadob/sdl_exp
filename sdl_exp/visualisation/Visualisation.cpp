#include "Visualisation.h"

#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.inl>

#include "GLcheck.h"
#include "interface/Scene.h"

#include "Text.h"

#define FOVY 60.0f
#define NEAR_CLIP 0.005f
#define FAR_CLIP 500.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f
#define SHIFT_MULTIPLIER 5.0f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_MOVE 0.1f
#define DELTA_STRAFE 0.1f
#define DELTA_ASCEND 0.1f
#define DELTA_ROLL 0.01f
#define ONE_SECOND_MS 1000
#define VSYNC 1

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

Visualisation::Visualisation(char *windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT)
    : hud(windowWidth, windowHeight)
    , camera(glm::vec3(50,50,50))
    , scene(0)
    , isInitialised(false)
    , continueRender(true)
	, msaaState(true)
    , windowTitle(windowTitle)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , fpsDisplay(0)
{
    this->isInitialised = this->init();

    fpsDisplay = std::make_shared<Text>("", 10, glm::vec3(1.0f), Stock::Font::ARIAL);
    fpsDisplay->setUseAA(false);
    hud.add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::West, 0, 0, INT_MAX);
    helpText = std::make_shared<Text>("Controls\nW,S: Move Forwards/Backwards\nA,D: Strafe\nQ,E: Roll\nF1:  Toggle Show Controls\nF5:  Reload Resources/Shaders\nF8:  Toggle Show FPS\nF9:  Toggle Show Skybox\nF10: Toggle MSAA\nF11: Toggle Fullscreen\nESC: Quit", 20, glm::vec3(1.0f), Stock::Font::LUCIDIA_CONSOLE);
    helpText->setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));
    helpText->setVisible(false);
    hud.add(helpText, HUD::AnchorV::Center, HUD::AnchorH::Center, 0, 0, INT_MAX);
}
Visualisation::~Visualisation(){}
bool Visualisation::init(){
    SDL_Init(SDL_INIT_VIDEO);

    // Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    //Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

    this->window = SDL_CreateWindow
        (
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowWidth,
        this->windowHeight,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
        );

    if (this->window == NULL){
        printf("window failed to init");
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
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glShadeModel(GL_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);
        GL_CALL(glBlendEquation(GL_FUNC_ADD));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
        this->camera.turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
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
		this->hud.reload();
        break;
    default:
        // Do nothing?
        break;
    }
}
void Visualisation::close(){
    //Delete objects before we delete the GL context!
    fpsDisplay.reset();
    this->hud.clear();
    if (this->scene)
    {
        this->scene.reset();
    }
    SDL_GL_DeleteContext(this->context);
    SDL_DestroyWindow(this->window);
    this->window = nullptr;
    SDL_Quit();
}
void Visualisation::render()
{
    //Static fn var for tracking the time to send to scene->update()
    static unsigned int updateTime = 0;
    SDL_Event e;
    // Handle continuous key presses (movement)
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float turboMultiplier = state[SDL_SCANCODE_LSHIFT] ? SHIFT_MULTIPLIER : 1.0f;
    if (state[SDL_SCANCODE_W]) {
        this->camera.move(DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_A]) {
        this->camera.strafe(-DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_S]) {
        this->camera.move(-DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_D]) {
        this->camera.strafe(DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_Q]) {
        this->camera.roll(-DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_E]) {
        this->camera.roll(DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        this->camera.ascend(DELTA_ASCEND*turboMultiplier);
    }
    if (state[SDL_SCANCODE_LCTRL]) {
        this->camera.ascend(-DELTA_ASCEND*turboMultiplier);
    }

    // handle each event on the queue
    while (SDL_PollEvent(&e) != 0){
        switch (e.type){
        case SDL_QUIT:
            this->quit();
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
    unsigned int t_updateTime = SDL_GetTicks();
    //If the program runs for over ~49 days, the return value of SDL_GetTicks() will wrap
    if (t_updateTime < updateTime)
    {
        this->scene->_update(t_updateTime + (UINT_MAX - updateTime));
    }
    else
    {
        this->scene->_update(t_updateTime - updateTime);
    }
    updateTime = t_updateTime;
    // render
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glClearColor(0, 0, 0, 1));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    this->scene->_render();
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	this->hud.render();

    GL_CHECK();

    // update the screen
    SDL_GL_SwapWindow(window);
}
void Visualisation::run(){
    if (!this->isInitialised){
        printf("Visulisation not initialised yet.");
    }
    else if (!this->scene){
        printf("Scene not yet set.");
    }
    else {
        SDL_ShowWindow(this->window);
        SDL_StartTextInput();
        this->continueRender = true;
        while (this->continueRender){
            // Update the fps in the window title
            this->updateFPS();

            this->render();
        }
        SDL_StopTextInput();

    }

    this->close();
}
void Visualisation::defaultProjection(){
    glEnable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->frustum));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->camera.view()));
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
}
void Visualisation::quit(){
    this->continueRender = false;
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
    SDL_GL_GetDrawableSize(this->window, &this->windowWidth, &this->windowHeight);

    float fAspect = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
    double fovy = FOVY;

    //glViewport(0, 0, this->windowWidth, this->windowHeight);//This is now called by the scene each frame (because scaled render textures must call at their desired scale)
    float top = static_cast<float>(tan(glm::radians(fovy * 0.5)) * NEAR_CLIP);
    float bottom = -top;
    float left = fAspect * bottom;
    float right = fAspect * top;
    this->frustum = glm::frustum<float>(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
	//Notify other elements
    this->hud.resizeWindow(this->windowWidth, this->windowHeight);
    this->scene->_resize(this->windowWidth, this->windowHeight);
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
const Camera *Visualisation::getCamera() const{
    return &this->camera;
}
std::weak_ptr<Scene> Visualisation::getScene() const{
    return this->scene;
}
const glm::mat4 *Visualisation::getFrustrumPtr() const{
    return &this->frustum;
}
HUD* Visualisation::getHUD(){
	return &hud;
}
