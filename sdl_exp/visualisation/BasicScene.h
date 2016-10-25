#ifndef __BasicScene_h__
#define __BasicScene_h__
#include "interface/Scene.h"
#include "Skybox.h"
#include "Axis.h"
#include "Visualisation.h"
/**
 * This class provides a baseclass for Scenes which only require single pass rendering
 * Natively includes a Skybox and Axis
 * Allows registering of entities, so their shaders can be reloaded on reload events
 */
class BasicScene : protected Scene
{
public:
	BasicScene(Visualisation& vis);
protected:
	/**
	 * Registers an entity, so the scene can setup it's modelview and projection matrices and trigger reloads
	 */
	void registerEntity(std::shared_ptr<Entity> ent);
	/**
	 * Override this method and do your rendering calls here
	 */
	virtual void render() = 0;
	/**
	 * Called when keys are pressed
	 * @param keycode The key that was pressed
	 * @param x X coordinate of the mouse when the keypress occurred
	 * @param y Y coordinate of the mouse when the keypress occurred
	 * @return True if you wish to allow the keypress to be handled elsewhere
	 */
	virtual bool keypress(SDL_Keycode keycode, int x, int y){ return true; };
	/**
	 * Called when the window is resized
	 * @param width The new window width
	 * @param height The new window height
	 */
	virtual void resize(int width, int height){};
	/**
	 * Called when a reload event occurs
     */
	virtual void reload(){};
	/**
	 * Called when you should update your scene
	 * @param frameTime The number of milliseconds since the last time this method was called
	 */
	virtual void update(unsigned int frameTime){};
	/**
	 * Toggles whether the skybox should be used or not
	 * @param state The desired skybox state
	 */
	void setSkybox(bool state);
	/**
	 * Toggles whether the axis should be rendered or not
	 * @param state The desired axis rendering state
	 */
	void setRenderAxis(bool state);
    
    Axis axis;
    std::unique_ptr<Skybox> skybox;
	bool renderAxisState;
private:
	/**
	 * Internal render functionality, calls render()
	 */
	void _render() override final;
	/**
	 * Internal keypress functionality, calls keypress()
	 */
	bool _keypress(SDL_Keycode keycode, int x, int y) override final;
	/**
	 * Internal resize functionality, calls resize()
	 */
	inline virtual void _resize(int width, int height) override final { resize(width, height); };
	/**
	 * Internal reload functionality, calls reload()
	 */
	void _reload() override final;
	/**
	 * Internal update functionality, calls update()
	 */
	inline virtual void _update(unsigned int frameTime) override final { update(frameTime); };
	/**
	 * Holds registered entities so the BasicScene can automatically reload them
	 */
	std::vector<std::shared_ptr<Entity>> entities;
	/**
	 * Provides a simple default lighting configuration located at the camera using the old fixed function pipeline methods
	 */
    void defaultLighting();
};
#endif //__BasicScene_h__