#ifndef __VisualisationVR_h__
#define __VisualisationVR_h__

#include <memory>
#include <thread>
#include <atomic>
#include <SDL/SDL.h>
#include <glm/glm.hpp>
#include <vector>

#include <openvr/openvr.h>

#include "../interface/Viewport.h"
#include "../camera/NoClipCamera.h"
#include "../multipass/FrameBuffer.h"
#include "../Sprite2D.h"
#include "../HUD.h"
#include "../Entity.h"
#include "HMDCamera.h"
#include "TrackedDevicesVR.h"
#include "SceneVR.h"

#undef main //SDL breaks the regular main entry point, this fixes

class Scene;
class Text;

/**
* This class provides an OpenGL window
* @todo Can we instead return weak pointers to camera/hud?
* @todo Add a key handler interface
* @todo Add support for generic camera classes
* @todo Make a runAsync() variation
*/
class VisualisationVR : public ViewportExt
{
public:

    /**
    * Creates a new window providing OpenGL functionality
    * @param windowTitle The title of the window
    * @param windowWidth The width of the contained graphics panel
    * @param windowHeight The height of the contained graphics panel
    */
    VisualisationVR(char *windowTitle, int windowWidth, int windowHeight);
    /**
    * Default destructor, destruction happens in close() to ensure objects are killed before the GL context
    * @see close()
    */
    ~VisualisationVR();
    /**
    * Renders a single frame
    * Also handle user inputs
    * @note This only need be called if manually triggering frame updates, run() should otherwise be used
    */
    void render();
    /**
    * Executes the render loop, this is a blocking call
    * @see quit() to externally kill the loop
    */
    void run();
    /**
    * Executes the redner loop in an asynchronous background thread
    * @see quit() to externally kill the loop
    */
    void runAsync();
    /**
    * Returns whether the window is currently rendering
    * @note This may report false early if the rendering is in the process of exiting
    */
    bool isRunning() const { return continueRender; }
    /**
    * @return The current window title
    */
    const char *getWindowTitle() const override;
    /**
    * Sets the window title
    * @param windowTitle Desired title of the window
    */
    void setWindowTitle(const char *windowTitle) override;
    /**
    * Sets a flag telling the render loop to exit
    * @note run() should be called to start the render loop
    */
    void quit();
    /**
    * Toggles the window between borderless fullscreen and windowed states
    */
    void toggleFullScreen();
    /**
    * @return True if the window is currently full screen
    */
    bool isFullscreen() const;
    /**
    * Returns a const pointer to the visualisation's Camera
    * @return The camera
    */
    std::shared_ptr<const Camera> getCamera() const override;
    /**
     * Sets the Scene object to be rendered within the viewport
     * @return The previously bound Scene (or SceneVR)
     * @note Calling getScene() will return a weak_ptr<Scene> to the object you provide
     */
    std::shared_ptr<Scene> setScene(std::unique_ptr<Scene> scene);
    /**
    * Returns a pointer to the visualisation's Scene
    * @return The scene
    * @note Preventing the Vis from deleting the Scene will cause GL errors to occur
    */
    std::weak_ptr<Scene> getScene() const;
    /**
    * Returns a constant pointer to the visualisations view frustum
    * This pointer can be used to continuously track the visualisations projection matrix
    * @return A pointer to the projection matrix
    */
	const glm::mat4 *getProjectionMatPtr() const override;
    glm::mat4 getProjectionMat() const override;
    /**
    * Returns the visusalisation's HUD, to be used to add overlays
    * @return The visualisation's scene
    */
    std::weak_ptr<HUD> getHUD() override;
    /**
    * @return The current viewport dimensions
    */
	glm::uvec2 getWindowDims() const override { return companionWindowDims; }
    
    ViewportExt &toViewportExt() { return reinterpret_cast<ViewportExt &>(*this); }
private:
    friend class SceneVR;
    void setVR(SceneVR *scene);
    /**
    * Provides key handling for none KEY_DOWN events of utility keys (ESC, F11, F10, F5, etc)
    * @param keycode The keypress detected
    * @param x The horizontal mouse position at the time of the KEY_DOWN event
    * @param y The vertical mouse position at the time of the KEY_DOWN event
    * @note Unsure whether the mouse position is relative to the window
    */
    void handleKeypress(SDL_Keycode keycode, int x, int y);
    /**
    * Moves the camera according to the motion of the mouse (whilst the mouse is attatched to the window via toggleMouseMode())
    * @param x The horizontal distance moved
    * @param y The vertical distance moved
    * @note This is called within the render loop
    */
    void handleMouseMove(int x, int y);
    /**
    * Simple implementation of an FPS counter
    * @note This is called within the render loop
    */
    void updateFPS();
    /**
    * Updates the viewport and projection matrix
    * This should be called after window resize events, or simply if the viewport needs generating
    */
    void resizeWindow();
    /**
    * Initialises SDL and creates the window
    * @return Returns true on success
    * @note This method doesn't begin the render loop, use run() for that
    */
    bool init();
    /**
    * Provides destruction of the object, deletes child objects, removes the GL context, closes the window and calls SDL_quit()
    */
    void close();
    /**
    * Internal run method, allows us to avoid starting the render loop if it's already executing when run() or runAsync() are called
    */
    void _run();
    /**
    * Kills thread t, only to be called from host threead.
    */
    void killThread();
    std::thread *t;
    SDL_Window* window;
    SDL_Rect windowedBounds;
    SDL_GLContext context;

    std::shared_ptr<HUD> hud;
    std::shared_ptr<Scene> scene;
    SceneVR *sceneVR;

    bool isInitialised;
    std::atomic<bool> continueRender;

    //FPS tracking stuff
    unsigned int previousTime = 0;
    unsigned int currentTime;
    unsigned int frameCount = 0;
    std::shared_ptr<Text> fpsDisplay;

    //OpenVR items
    vr::IVRSystem *vr_HMD;                  //HMD configuration access
    glm::uvec2 vr_renderTargetDimensions;
    std::shared_ptr<FrameBuffer> vr_leftRenderFB, vr_rightRenderFB;
    std::shared_ptr<FrameBuffer> vr_leftResolveFB, vr_rightResolveFB;

	std::shared_ptr<TrackedDevicesVR> vr_renderModels;

	const char* companionWindowTitle;
	glm::uvec2 companionWindowDims;
	std::shared_ptr<Sprite2D> companionLeft;
	std::shared_ptr<Sprite2D> companionRight;
    void renderStereoTargets();
};

#endif //ifndef __VisualisationVR_h__
