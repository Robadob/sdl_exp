#include "Visualisation.h"
#include <math.h>
#include <string>
#include <sstream>

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

Visualisation::Visualisation(char* windowTitle, int windowWidth, int windowHeight)
    : isInitialised(false)
    , quit(false)
    , windowTitle(windowTitle)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , camera(glm::vec3(10,10,10))
    , renderAxisState(false)
    , axis(0.5)
    , msaaState(true)
{
    this->isInitialised = this->init();
    skybox = new Skybox();
}

Visualisation::~Visualisation(){
}


bool Visualisation::init(){
    bool result = true;

    SDL_Init(SDL_INIT_VIDEO);

    // Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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

        //Enable VSync
        int swapIntervalResult = SDL_GL_SetSwapInterval(VSYNC);
        if (swapIntervalResult == -1){
            printf("Swap Interval Failed: %s\n", SDL_GetError());
        }

        // Init glew.
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            exit(1);
        }

        // Create the scene - need to be done after glew is init
        this->scene = new VisualisationScene(&this->camera);
        

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
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F5:
        this->skybox->reload();
        this->scene->reload();
        break;
    default:
        // Do nothing?
        break;
    }
}

void Visualisation::setMSAA(bool state)
{
    this->msaaState = state;
    if (this->msaaState)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void Visualisation::close(){
    //Delete objects before we delete the GL context!
    delete this->scene;
    delete this->skybox;
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
            this->clearFrame();
            this->skybox->render(&camera, this->frustum);
            this->defaultProjection();
            if (this->renderAxisState)
                this->axis.render();
            this->defaultLighting();
            this->scene->render(this->frustum);
            // check for GL errors
            int err;
            if ((err = glGetError()) != GL_NO_ERROR)
            {
                //const char* message = (const char*)gluErrorString(err);
                //fprintf(stderr, "OpenGL Error Occured : %s\n", message
                printf("OpenGL Error Occured: %i\n", err);// : %s\n", message);
            }
            // update the screen
            SDL_GL_SwapWindow(window);
        }
        SDL_StopTextInput();

    }

    this->close();
}
void Visualisation::clearFrame()
{
    glViewport(0, 0, this->windowWidth, this->windowHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Visualisation::defaultProjection()
{
    glEnable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->frustum));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(this->camera.view()));
}
void Visualisation::defaultLighting()
{
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
void Visualisation::renderAxis()
{
    glPushMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //Axis
        glLineWidth(1);
        glPushMatrix();
            glColor4f(1.0, 1.0, 1.0, 1.0);//White-x
            glBegin(GL_LINES);
                glVertex3f(0, 0, 0);
                glVertex3f(100, 0, 0);
            glEnd();
        glPopMatrix();
        glPushMatrix();
            glColor4f(0.0, 1.0, 0.0, 1.0);//Green-y
            glBegin(GL_LINES);
                glVertex3f(0, 0, 0);
                glVertex3f(0, 100, 0);
            glEnd();
        glPopMatrix();
        glPushMatrix();
            glColor4f(0.0, 0.0, 1.0, 1.0);//Blue-z
            glBegin(GL_LINES);
                glVertex3f(0, 0, 0);
                glVertex3f(0, 0, 100);
            glEnd();
        glPopMatrix();
    glPopMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void Visualisation::setRenderAxis(bool state)
{
    this->renderAxisState = state;
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

void Visualisation::handleMouseMove(int x, int y){
    if (SDL_GetRelativeMouseMode()){
        this->camera.turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
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
        newTitle << this->windowTitle << " (" << std::to_string(static_cast<int>(std::ceil(fps))) << " fps)";
        SDL_SetWindowTitle(this->window, newTitle.str().c_str());

        // reset values;
        this->previousTime = this->currentTime;
        this->frameCount = 0;
    }
}

Camera *Visualisation::getCamera() 
{
    return &this->camera;
}
VisualisationScene *Visualisation::getScene() const
{
    return this->scene;
}
