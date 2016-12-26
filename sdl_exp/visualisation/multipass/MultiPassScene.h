#ifndef __MultiPassScene_h__
#define __MultiPassScene_h__

#include <memory>
#include <vector>
#include "../interface/Scene.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include "../Visualisation.h"
#include "RenderPass.h"

/**
 * This Scene implementation allows each render to occur over multiple passes
 * Providing the ability to perform render to texture operations
 */
class MultiPassScene : public Scene
{
public:
	/**
	 * Creates a new MultiPassScene
	 * @param vis The visualisation that is hosting the scene.
	 */
	MultiPassScene(Visualisation &vis);
    /**
     * Called once per frame when Scene animation calls should be executed
     * @param frameTime The number of miliseconds since update was last called
     */
    virtual void update(unsigned int frameTime){};
	/**
	 * Adds a renderPass to the render loop
	 * @param index The position which it should be rendered at (low->high)
	 * @param rp A shared pointer to the RenderPass
	 */
    std::shared_ptr<RenderPass> addPass(int index, std::shared_ptr<RenderPass> rp);
    /**
     * Called when the user requests a reload
     * @note You should call functions such as shaders->reload() here
     */
	virtual void reload() = 0;
	/**
	 * Called when the user presses a keyboard key
	 * @param keycode The SDL_Keycode representing the pressed key (defined in the form SDLK_<key>)
	 * @param x The mouse's current x coordinate relative to the window
	 * @param y The mouse's current y coordinate relative to the window
	 * @return Returning true permits the visualisation to also handle the keypress
	 */
	virtual bool keypress(SDL_Keycode keycode, int x, int y) = 0;
protected:
	/**
	 * Registers an entity, so the scene can manage it's modelview and projection matrices and reloads
	 */
	virtual void registerEntity(std::shared_ptr<Renderable> ent) final;
private:
	/**
	 * Called once per frame when Scene render calls should be executed
	 * Triggers all of the RenderPass's to render in sequence
	 */
	void _render() override final;
	/**
	 * Called when the user presses a keyboard key
	 * @param keycode The SDL_Keycode representing the pressed key (defined in the form SDLK_<key>)
	 * @param x The mouse's current x coordinate relative to the window
	 * @param y The mouse's current y coordinate relative to the window
	 * @return Returning true permits the visualisation to also handle the keypress
	 * @note Calls keypress()
	 */
	virtual bool _keypress(SDL_Keycode keycode, int x, int y) override final { return keypress(keycode, x, y); };
	/**
	 * Resizes the textures attatched the Framebuffers within RenderPass's
	 */
	virtual void _resize(int width, int height) override final;
	/**
	 * Reloads all registered entities, then calls reload on the subclassed scene
	 */
	virtual void _reload() override final;
	/**
	 * Internal update functionality, calls update()
	 */
	inline virtual void _update(unsigned int frameTime) override final { update(frameTime); };
	/**
	* Holds registered entities so the Scene can automatically reload them
	*/
	std::vector<std::shared_ptr<Renderable>> entities;
	/**
	* Holds registered render passes so they can be triggered during render
	*/
    std::map<int, std::shared_ptr<RenderPass>> rpMap;
};

#endif //ifndef __MultiPassScene_h__