#ifndef __Visualisation_h__
#define __Visualisation_h__

#include <SDL/SDL.h>
#include <glm/glm.hpp>

#include "interface/Viewport.h"
#include <memory>
#include "Camera.h"
#include "HUD.h"

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
class Visualisation : public Viewport
{
public:

	/**
	 * Creates a new window providing OpenGL functionality
	 * @param windowTitle The title of the window
	 * @param windowWidth The width of the contained graphics panel
	 * @param windowHeight The height of the contained graphics panel
	 */
	Visualisation(char *windowTitle, int windowWidth, int windowHeight);
	/**
	 * Default destructor, destruction happens in close() to ensure objects are killed before the GL context
	 * @see close()
	 */
    ~Visualisation();
	/**
	 * Renders a single frame
	 * Also handle user inputs
	 * @note This only need be called if manually triggering frame updates, run() should otherwise be used
	 */
    void Visualisation::render();
	/**
	 * Executes the render loop, this is a blocking call
     * @see quit() to externally kill the loop
     * @todo Make a runAsync() method
	 */
	void run();
	/**
	 * @return The current window title
	 */
    const char *getWindowTitle() const;
	/**
	 * Sets the window title
	 * @param windowTitle Desired title of the window
	 */
    void setWindowTitle(const char *windowTitle);
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
	 * Toggles whether the mouse is hidden and returned relative to the window
	 */
    void toggleMouseMode();
	/**
	 * Loads the ModelView and Projection matrices using the old fixed function pipeline methods
	 */
	void defaultProjection();
	/**
	 * Toggles whether Multi-Sample Anti-Aliasing should be used or not
	 * @param state The desired MSAA state
	 * @note Unless blocked by the active Scene the F10 key toggles MSAA at runtime
	 */
	void setMSAA(bool state);
	/**
	 * Returns a const pointer to the visualisation's Camera
	 * @return The camera
	 */
	const Camera *getCamera() const;
	/**
	 * Sets the Scene object to be rendered within the viewport
	 * @return The previously bound Scene
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
	const glm::mat4 *getFrustrumPtr() const override;
	/**
	 * Returns the visusalisation's HUD, to be used to add overlays
	 * @return The visualisation's scene
	 */
	HUD* getHUD();
	/**
	 * @return The current window/viewport width
	 */
	const int& getWindowWidth() const { return windowWidth; }
	/**
	 * @return The current window/viewport height
	 */
	const int& getWindowHeight() const { return windowHeight; }
private:
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

    SDL_Window* window;
    SDL_Rect windowedBounds;
    SDL_GLContext context;

	HUD hud;
    Camera camera;
	std::shared_ptr<Scene> scene;
    glm::mat4 frustum;

    bool isInitialised;
    bool continueRender;

    bool msaaState;

    const char* windowTitle;
    int windowWidth;
    int windowHeight;

    //FPS tracking stuff
    unsigned int previousTime = 0;
    unsigned int currentTime;
    unsigned int frameCount = 0;
    std::shared_ptr<Text> fpsDisplay;
    std::shared_ptr<Text> helpText;
};

#endif //ifndef __Visualisation_h__
