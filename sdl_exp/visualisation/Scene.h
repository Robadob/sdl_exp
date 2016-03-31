#ifndef __Scene_h__
#define __Scene_h__

#include <memory>
#include <vector>

#include "Entity.h"
#include "Shaders.h"
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