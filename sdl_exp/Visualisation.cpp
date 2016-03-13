#include "Visualisation.h"

#include <sstream>
#include <glm\gtc\type_ptr.hpp>

#include "GLcheck.h"

#define FOVY 60.0f
#define NEAR_CLIP 0.001f
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

/*
Creates a new window providing OpenGL functionality
@param windowTitle The title of the window
@param windowWidth The width of the contained graphics panel
@param windowHeight The height of the contained graphics panel
*/
Visualisation::Visualisation(char *windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT)
    : isInitialised(false)
    , continueRender(true)
    , windowTitle(windowTitle)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , camera(glm::vec3(10,10,10))
    , renderAxisState(false)
    , axis(0.5)
    , msaaState(true)
    , skybox(0)
{
    this->isInitialised = this->init();
}
/*
Default destructor, destruction happens in close() to ensure objects are killed before the GL context
@see close()
*/
Visualisation::~Visualisation(){}
/*
Initialises SDL and creates the window
@return Returns true on success
@note This method doesn't begin the render loop, use run() for that
*/
bool Visualisation::init(){
    bool result = true;

    SDL_Init(SDL_INIT_VIDEO);

    // Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    //Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

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

        // Create the scene - need to be done after GL context has been created
        this->scene = new VisualisationScene(this);
        
        // Setup gl stuff
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glShadeModel(GL_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);
        setMSAA(this->msaaState);

        // Setup the projection matrix
        this->resizeWindow();
        GL_CHECK();
        return true;
    }
    return false;
}
/*
Moves the camera according to the motion of the mouse (whilst the mouse is attatched to the window via toggleMouseMode())
@param x The horizontal distance moved
@param y The vertical distance moved
@note This is called within the render loop
*/
void Visualisation::handleMouseMove(int x, int y){
    if (SDL_GetRelativeMouseMode()){
        this->camera.turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
    }
}
/*
Provides key handling for none KEY_DOWN events of utility keys (ESC, F11, F10, F5, etc)
@param keycode The keypress detected
@param x The horizontal mouse position at the time of the KEY_DOWN event
@param y The vertical mouse position at the time of the KEY_DOWN event
@note Unsure whether the mouse position is relative to the window
*/
void Visualisation::handleKeypress(SDL_Keycode keycode, int x, int y){

    switch (keycode){
    case SDLK_ESCAPE:
        this->quit();
        break;
    case SDLK_F11:
        this->toggleFullScreen();
        break;
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F9:
        this->setSkybox(!this->skybox);
        break;
    case SDLK_F5:
        if (this->skybox)
            this->skybox->reload();
        this->scene->reload();
        break;
    default:
        // Do nothing?
        break;
    }
}
/*
Provides destruction of the object, deletes child objects, removes the GL context, closes the window and calls SDL_quit()
*/
void Visualisation::close(){
    //Delete objects before we delete the GL context!
    if (this->scene)
    {
        delete this->scene;
        this->scene = 0;
    }
    if (this->skybox)
    {
        delete this->skybox;
        this->skybox = 0;
    }
    SDL_GL_DeleteContext(this->context);
    SDL_DestroyWindow(this->window);
    this->window = 0;
    SDL_Quit();
}
/*
Executes the render loop
@see setQuit() to externally kill the loop
*/
void Visualisation::run(){
    if (!this->isInitialised){
        printf("Visulisation not initialised yet.");
    }
    else {
        SDL_Event e;
        SDL_StartTextInput();
        this->continueRender = true;
        while (this->continueRender){
            // Update the fps in the window title
            this->updateFPS();

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
            this->scene->update();
            // render
            this->clearFrame();
            if (this->skybox)
                this->skybox->render(&camera, this->frustum);
            this->defaultProjection();
            if (this->renderAxisState)
                this->axis.render();
            this->defaultLighting();
            this->scene->render(this->frustum);

            GL_CHECK();

            // update the screen
            SDL_GL_SwapWindow(window);
        }
        SDL_StopTextInput();

    }

    this->close();
}
/*
Clears the frame
@note This is called within the render loop before the frame is rendered
*/
void Visualisation::clearFrame(){
    glViewport(0, 0, this->windowWidth, this->windowHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
/*
Loads the ModelView and Projection matrices using the old fixed function pipeline methods
*/
void Visualisation::defaultProjection(){
    glEnable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->frustum));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->camera.view()));
}
/*
Provides a simple default lighting configuration located at the camera using the old fixed function pipeline methods
*/
void Visualisation::defaultLighting(){
    glEnable(GL_LIGHT0);
    glm::vec3 eye = this->camera.getEye();
    float lightPosition[4] = { eye.x, eye.y, eye.z, 1 };
    float amb[4] = { 0.8f, 0.8f, 0.8f, 1 };
    float diffuse[4] = { 0.2f, 0.2f, 0.2f, 1 };
    float white[4] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    // Spotlight stuff
    //float angle = 180.0f;
    //glm::vec3 look = this->camera.getLook();
   // float direction[4] = { look.x, look.y, look.z, 0 };
    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
}
/*
Toggles whether the skybox should be used or not
@param state The desired skybox state
*/
void Visualisation::setSkybox(bool state){
    if (state&&!this->skybox)
        this->skybox = new Skybox(this->getCamera()->getSkyboxViewMatPtr(), this->getFrustrumPtr());
    else if (!state&&this->skybox)
    {
        delete this->skybox;
        this->skybox = 0;
    }
}
/*
Toggles whether Multi-Sample Anti-Aliasing should be used or not
@param state The desired MSAA state
*/
void Visualisation::setMSAA(bool state){
    this->msaaState = state;
    if (this->msaaState)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}
/*
Toggles whether the axis should be rendered or not
@param state The desired axis rendering state
*/
void Visualisation::setRenderAxis(bool state){
    this->renderAxisState = state;
}
/*
@return The current window title (sans the FPS)
*/
const char *Visualisation::getWindowTitle() const{
    return this->windowTitle;
}
/*
Sets the window title (sans the FPS)
@param windowTitle Desired title of the window
*/
void Visualisation::setWindowTitle(const char *windowTitle){
    this->windowTitle = windowTitle;
}
/*
Sets a flag telling the render loop to exit
*/
void Visualisation::quit(){
    this->continueRender = false;
}
/*
Toggles the window between borderless fullscreen and windowed states
*/
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
/*
Toggles whether the mouse is hidden and returned relative to the window
*/
void Visualisation::toggleMouseMode(){
    if (SDL_GetRelativeMouseMode()){
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
}
/*
Updates the viewport and projection matrix
This should be called after window resize events, or simply if the viewport needs generating
*/
void Visualisation::resizeWindow(){
    // Use the sdl drawable size
    SDL_GL_GetDrawableSize(this->window, &this->windowWidth, &this->windowHeight);

    float fAspect = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
    double fovy = FOVY;

    glViewport(0, 0, this->windowWidth, this->windowHeight);
    float top = static_cast<float>(tan(glm::radians(fovy * 0.5)) * NEAR_CLIP);
    float bottom = -top;
    float left = fAspect * bottom;
    float right = fAspect * top;
    this->frustum = glm::frustum<float>(left, right, bottom, top, NEAR_CLIP, FAR_CLIP);
}
/*
@return True if the window is currently full screen
*/
bool Visualisation::isFullscreen() const{
    // Use window borders as a toggle to detect fullscreen.
    return (SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS;
}
/*
Simple implementation of an FPS counter
Appends the fps to the windows title
@note This is called within the render loop
*/
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
        newTitle << this->windowTitle << " (" << std::to_string(static_cast<int>(std::ceil(fps))) << " fps)";
        SDL_SetWindowTitle(this->window, newTitle.str().c_str());

        // reset values;
        this->previousTime = this->currentTime;
        this->frameCount = 0;
    }
}
/*
Returns a const pointer to the visualisation's Camera
@return The camera
*/
const Camera *Visualisation::getCamera() const{
    return &this->camera;
}
/*
Returns a pointer to the visualisation's Scene
@return The scene
*/
const VisualisationScene *Visualisation::getScene() const{
    return this->scene;
}
/*
Returns a constant pointer to the visualisations view frustum
This pointer can be used to continuously track the visualisations projection matrix
@return A pointer to the projection matrix
*/
const glm::mat4 *Visualisation::getFrustrumPtr() const{
    return &this->frustum;
}