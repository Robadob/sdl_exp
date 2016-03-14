#ifndef __EntityScene_h__
#define __EntityScene_h__

#include "visualisation/Scene.h"
#include "visualisation/Entity.h"

/*
Example scene to demonstrate how to use SDL_EXP
*/
class EntityScene : Scene
{
public:
    EntityScene(Visualisation &visualisation);

    void render() override;
    void reload() override;
    void update() override;
private:
    Entity icosphere;
    float tick;
    ~EntityScene() override;//Private to prevent stack allocation
};

#endif
