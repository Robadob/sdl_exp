#include "VisualisationVR.h"

#include "util/GLcheck.h"
#include "Visualisation.h"
#include "interface/Scene.h"

#define VSYNC 0 //Disabled in SteamVR sample

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

VisualisationVR::VisualisationVR(char *windowTitle, int windowWidth = DEFAULT_WINDOW_WIDTH, int windowHeight = DEFAULT_WINDOW_HEIGHT)
    : t(nullptr)
    , hud(std::make_shared<HUD>(windowWidth, windowHeight))
    , scene(nullptr)
    , isInitialised(false)
    , continueRender(false)
    , msaaState(true)
    , windowTitle(windowTitle)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , fpsDisplay(nullptr)
    , vr_HMD(nullptr)
    , vr_renderModels(nullptr)
    , vr_driverString("No Driver")
    , vr_displayString("No Display")
{
    this->isInitialised = this->init();

    //Can we do vr text?
    //fpsDisplay = std::make_shared<Text>("", 10, glm::vec3(1.0f), Stock::Font::ARIAL);
    //fpsDisplay->setUseAA(false);
    //hud.add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::West, 0, 0, INT_MAX);    
}
VisualisationVR::~VisualisationVR()
{
    this->close();
}

bool VisualisationVR::init()
{    
    //Create SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    //Load SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    vr_HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None)
    {
        vr_HMD = nullptr;
        printf("Unable to init VR runtime: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        return false;
    }

    //Configure GL context
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    //MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0); //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    //Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#ifdef _DEBUG
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    //Create companion window
    this->window = SDL_CreateWindow
        (
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowWidth,
        this->windowHeight,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL //| SDL_WINDOW_BORDERLESS
        );

    if (!this->window){
        printf("Window failed to init: %s\n", SDL_GetError());
        return false;
    }
    SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
    SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);

    // Get context
    this->context = SDL_GL_CreateContext(window);
    if (!this->context)
    {
        printf("OpenGL context could not be created: %s\n", SDL_GetError());
        return false;
    }
    //Enable VSync
    if (SDL_GL_SetSwapInterval(VSYNC) == -1){
        printf("Unable to set VSync: %s\n", SDL_GetError());
    }

    GLEW_INIT();
	    
    //Setup HMD FrameBuffers
    vr_HMD->GetRecommendedRenderTargetSize(&vr_renderTargetDimensions.x, &vr_renderTargetDimensions.y);
    //Render in Multisample
    vr_leftRenderFB = std::make_shared<FrameBuffer>(vr_renderTargetDimensions, FBAFactory::ManagedColorTexture(GL_RGBA8, GL_RGBA), FBAFactory::ManagedDepthRenderBuffer(),FBAFactory::Disabled(), 4);
    vr_rightRenderFB = std::make_shared<FrameBuffer>(vr_renderTargetDimensions, FBAFactory::ManagedColorTexture(GL_RGBA8, GL_RGBA), FBAFactory::ManagedDepthRenderBuffer(), FBAFactory::Disabled(), 4);
    //Resolve(?) to none multisample
    vr_leftResolveFB = std::make_shared<FrameBuffer>(vr_renderTargetDimensions, FBAFactory::ManagedColorTexture(GL_RGBA8, GL_RGBA), FBAFactory::Disabled());
    vr_rightResolveFB = std::make_shared<FrameBuffer>(vr_renderTargetDimensions, FBAFactory::ManagedColorTexture(GL_RGBA8, GL_RGBA), FBAFactory::Disabled());
    //Setup companion
    vr_companion = std::make_shared<CompanionVR>();
    //Load render models
    vr_renderModels = std::make_shared <TrackedDevicesVR>(vr_HMD);
    if (!vr_renderModels->getInitState())
    {
        vr::VR_Shutdown();
        vr_HMD = nullptr;
        return false;
    }


#ifdef _DEBUG
    //Callback format
    //void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
    //{
    //    printf("GL Error: %s\n", message);
    //}
    //glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
    // Setup gl stuff
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glCullFace(GL_BACK));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glShadeModel(GL_SMOOTH));
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GL_CALL(glEnable(GL_LIGHTING));
    GL_CALL(glEnable(GL_LIGHT0));
    GL_CALL(glEnable(GL_COLOR_MATERIAL));
    GL_CALL(glEnable(GL_NORMALIZE));
    GL_CALL(glBlendEquation(GL_FUNC_ADD));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    setMSAA(this->msaaState);

    // Setup the projection matrix
    this->resizeWindow();
    GL_CHECK();

    if (!vr::VRCompositor())
    {
        printf("Compositor initialization failed. See (OpenVR?) log file for details\n");
        return false;
    }

    return true;
}

std::shared_ptr<Scene> VisualisationVR::setScene(std::unique_ptr<Scene> scene)
{
    std::shared_ptr<Scene> oldScene = this->scene;
    this->scene = std::shared_ptr<Scene>(scene.release());
    return oldScene;
}

void VisualisationVR::run()
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
void VisualisationVR::_run()
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

void VisualisationVR::close(){
    killThread();

    //Shutdown HMD
    if (vr_HMD)
    {
        vr::VR_Shutdown();
        vr_HMD = nullptr;
    }
    //Purge Render Models
    vr_renderModels.reset();

    assert(this->window);//There should always be a window, it might just be hidden
    SDL_GL_MakeCurrent(this->window, this->context);
    //Delete objects before we delete the GL context!
    fpsDisplay.reset();
    this->hud.reset();
    if (this->scene)
    {
        this->scene.reset();
    }
    SDL_DestroyWindow(this->window);
    this->window = nullptr;
    SDL_GL_DeleteContext(this->context);
    SDL_Quit();
}

void VisualisationVR::render()
{
    //Static fn var for tracking the time to send to scene->update()
    static unsigned int updateTime = 0;
    SDL_Event e;
    // Handle continuous key presses (movement)
    //const Uint8 *state = SDL_GetKeyboardState(NULL);
    //float turboMultiplier = state[SDL_SCANCODE_LSHIFT] ? SHIFT_MULTIPLIER : 1.0f;
    //if (state[SDL_SCANCODE_W]) {
    //    this->camera.move(DELTA_MOVE*turboMultiplier);
    //}
    //if (state[SDL_SCANCODE_A]) {
    //    this->camera.strafe(-DELTA_STRAFE*turboMultiplier);
    //}
    //if (state[SDL_SCANCODE_S]) {
    //    this->camera.move(-DELTA_MOVE*turboMultiplier);
    //}
    //if (state[SDL_SCANCODE_D]) {
    //    this->camera.strafe(DELTA_STRAFE*turboMultiplier);
    //}
    //if (state[SDL_SCANCODE_Q]) {
    //    this->camera.roll(-DELTA_ROLL);
    //}
    //if (state[SDL_SCANCODE_E]) {
    //    this->camera.roll(DELTA_ROLL);
    //}
    //if (state[SDL_SCANCODE_SPACE]) {
    //    this->camera.ascend(DELTA_ASCEND*turboMultiplier);
    //}
    //if (state[SDL_SCANCODE_LCTRL]) {
    //    this->camera.ascend(-DELTA_ASCEND*turboMultiplier);
    //}

    // handle each event on the SDL queue
    while (SDL_PollEvent(&e) != 0){
        switch (e.type){
        case SDL_QUIT:
            this->quit();
            break;
        //case SDL_KEYDOWN:
        //{
        //    int x = 0;
        //    int y = 0;
        //    SDL_GetMouseState(&x, &y);
        //    this->handleKeypress(e.key.keysym.sym, x, y);
        //}
        //    break;
        //    //case SDL_MOUSEWHEEL:
        //    //break;
        //case SDL_MOUSEMOTION:
        //    this->handleMouseMove(e.motion.xrel, e.motion.yrel);
        //    break;
        //case SDL_MOUSEBUTTONDOWN:
        //    this->toggleMouseMode();
        //    break;

        }
    }

    //handle OpenVR events
	vr_renderModels->update();

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

    if (vr_HMD)
    {
		vr_renderModels->render();
        renderStereoTargets();
        //RenderCompanionWindow();
        vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)vr_leftResolveFB->getColorTextureName(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)vr_rightResolveFB->getColorTextureName(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
    }

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CALL(glClearColor(0, 0, 0, 1));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    this->scene->_render();
    GL_CALL(glViewport(0, 0, windowWidth, windowHeight));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    this->hud->render();

    GL_CHECK();

    // update the screen
    SDL_GL_SwapWindow(window);
}
void VisualisationVR::renderStereoTargets()
{
    //Render left eye multi-sample
    vr_leftRenderFB->use();
    GL_CALL(glViewport(0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y));
	this->vr_renderModels->getCamera()->useLeft();
    this->scene->_render();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    //Blit left eye
    GL_CALL(glDisable(GL_MULTISAMPLE));
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, vr_leftRenderFB->getFrameBufferName()));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, vr_leftResolveFB->getFrameBufferName()));

    GL_CALL(glBlitFramebuffer(0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y, 0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR));

    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    GL_CALL(glEnable(GL_MULTISAMPLE));

    //Render right eye multi-sample
    vr_rightRenderFB->use();
    GL_CALL(glViewport(0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y));
	this->vr_renderModels->getCamera()->useRight();
    this->scene->_render();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    //Blit left eye
    GL_CALL(glDisable(GL_MULTISAMPLE));
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, vr_rightRenderFB->getFrameBufferName()));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, vr_rightResolveFB->getFrameBufferName()));

    GL_CALL(glBlitFramebuffer(0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y, 0, 0, vr_renderTargetDimensions.x, vr_renderTargetDimensions.y,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR));

    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    GL_CALL(glEnable(GL_MULTISAMPLE));
}

std::shared_ptr<const Camera> VisualisationVR::getCamera() const
{
	if (this->vr_renderModels)
		return this->vr_renderModels->getCamera();
	return nullptr;
}
const glm::mat4 *VisualisationVR::getProjMatPtr() const{
	return this->vr_renderModels->getCamera()->getProjMatPtr();
}
const glm::mat4 VisualisationVR::getProjMat() const{
	return this->vr_renderModels->getCamera()->getProjMat();
}

std::weak_ptr<HUD> VisualisationVR::getHUD()
{
	return this->hud;
}

const char *VisualisationVR::getWindowTitle() const{
	return this->windowTitle;
}
void VisualisationVR::setWindowTitle(const char *windowTitle){
	this->windowTitle = windowTitle;
	SDL_SetWindowTitle(this->window, this->windowTitle);
}