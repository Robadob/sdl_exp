#ifndef __VisualisationScene_h__
#define __VisualisationScene_h__

#include <glm/glm.hpp>

#include "Shaders.h"
#include "Entity.h"

class Visualisation;

class VisualisationScene
{
public:
    VisualisationScene(Visualisation* vis = nullptr);
    ~VisualisationScene();

    void update();
    void reload();
    void render(glm::mat4 projection);
    void generate();

private:
    Visualisation *vis;
    Entity *ent;
    Shaders *shader;

};

#endif //ifndef __VisualisationScene_h__