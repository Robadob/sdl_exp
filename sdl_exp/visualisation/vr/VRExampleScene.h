#ifndef __VRExampleScene_h__
#define __VRExampleScene_h__
#include "../BasicScene.h"
#include "../VisualisationVR.h"
#include "SceneVR.h"
#include "../primitives/Points.h"
#include "../primitives/Lines.h"

#define MAX_SPHERES 10
#define SPHERE_RAD 0.1

class VRExampleScene : public BasicScene, public SceneVR
{
public:
    VRExampleScene::VRExampleScene(VisualisationVR &vis);

    void render() override;
    bool controllerEventVR(std::shared_ptr<Controller> controller, vr::EVRButtonId buttonId, vr::EVREventType buttonEvent) override;
private:
    std::shared_ptr<Entity> sphere[MAX_SPHERES]; 
    std::shared_ptr<Points> points;
    std::shared_ptr<Lines> lines;
    unsigned int heldSphere;
    glm::mat4 heldSphereMat4;
};

#endif //__VRExampleScene_h__