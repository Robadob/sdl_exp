#ifndef __Scene_h__
#define __Scene_h__
#include <SDL/SDL.h>

class Visualisation;

class Scene
{
public:
	/**
	 * Called by the viewport before swap buffers
	 * You should place all your rendering code here
	 * @note This method is named _render, such that subclasses can perform rendering management before deffering rendering to a similarly name method
	 */
	virtual void _render() = 0;
	/**
	 * Called when the user presses a keyboard key
	 * @param keycode The SDL_Keycode representing the pressed key (defined in the form SDLK_<key>)
	 * @param x The mouse's current x coordinate relative to the window
	 * @param y The mouse's current y coordinate relative to the window
	 * @return Returning true permits the visualisation to also handle the keypress
	 */
	virtual bool _keypress(SDL_Keycode keycode, int x, int y) { return true; };
	/**
	 * Called by the viewport when the windows dimensions change
	 */
	virtual void _resize(int width, int height) {};
	/**
	 * Called by the viewport when a reload is requested (F5 on the keyboard)
	 * @note This method is named _render, such that subclasses can perform internal reload before forwarding the reload call to a user controlled reload method
	 */
	virtual void _reload() {};
	/**
	 * Called by the viewport between render calls
	 * @param frameTime The number of milliseconds since update was last called
	 */
	virtual void _update(unsigned int frameTime) {};

protected:
	Scene(Visualisation &vis) :visualisation(vis){};
	Visualisation &visualisation;
	virtual ~Scene(){};
};

#endif //__Scene_h__