#ifndef __VRExampleScene_h__
#define __VRExampleScene_h__
#include "../BasicScene.h"
#include "../VisualisationVR.h"
#include "SceneVR.h"

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
};

#endif //__VRExampleScene_h__