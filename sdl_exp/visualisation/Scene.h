#ifndef __Scene_h__
#define __Scene_h__

#include "Shaders.h"

#include "Visualisation.h"

class Scene
{
public:

    /*
    Called once per frame when Scene animation calls should be executed
    */
    virtual void update() = 0;
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

protected:
    Scene(Visualisation &vis, Shaders *shaders);
    Visualisation &visualisation;
    Shaders *shaders;
    virtual ~Scene(){};//Protected destructor prevents this object being created on the stack (you must create via new)
};

#endif //ifndef __Scene_h__