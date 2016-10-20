#ifndef __Scene_h__
#define __Scene_h__

#include <memory>
#include <vector>

#include "Entity.h"
#include "shader/Shaders.h"
#include "Visualisation.h"

class Scene
{
    friend class Visualisation;
public:

    /*
    Called once per frame when Scene animation calls should be executed
    @param frameTime The number of miliseconds since update was last called
    */
    virtual void update(unsigned int frameTime) = 0;
    /*
    Called once per frame when Scene render calls should be executed
    */
    virtual void render() = 0;
    /*
    Manual destructor because the Scene MUST be destroyed prior to the Visualisation destroying the GL context
    */
    virtual void kill();
    /*
    Called when the user requests a reload
    @note You should call functions such as shaders->reload() here
    */
    virtual void reload() = 0;
    /*
    Called when the user presses a keyboard key
    @param keycode The SDL_Keycode representing the pressed key (defined in the form SDLK_<key>)
    @param x The mouse's current x coordinate relative to the window
    @param y The mouse's current y coordinate relative to the window
    @return Returning true permits the visualisation to also handle the keypress
    */
    virtual bool keypress(SDL_Keycode keycode, int x, int y) = 0;
protected:
    Scene(Visualisation &vis);
    Visualisation &visualisation;
    Shaders *shaders;
    void registerEntity(std::shared_ptr<Entity> ent);
    ~Scene(){};//Protected destructor prevents this object being created on the stack (you must create via new)
private:
    std::vector<std::shared_ptr<Entity>> entities;
    void _reload();
};

#endif //ifndef __Scene_h__