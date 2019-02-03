#ifndef __VRExampleScene_h__
#define __VRExampleScene_h__
#include "VisualisationVR.h"
#include "SceneVR.h"
#include "../BasicScene.h"
#include "../Draw.h"

#define MAX_SPHERES 10
#define SPHERE_RAD 0.1

class VRExampleScene : public BasicScene, public SceneVR
{
public:
    VRExampleScene::VRExampleScene(VisualisationVR &vis);

    void render() override;
    void update(const unsigned int &frameTime) override;
    bool controllerEventVR(std::shared_ptr<Controller> controller, vr::EVRButtonId buttonId, vr::EVREventType buttonEvent) override;
private:
    std::unique_ptr<Skybox> skyboxA, skyboxB;
    std::shared_ptr<Entity> portalStencil;
    std::shared_ptr<Entity> sphere[MAX_SPHERES]; 
    std::shared_ptr<Draw> pen;
    std::shared_ptr<PortableDraw> lance, pointer;
    unsigned int heldSphere;
    glm::mat4 heldSphereMat4;
};

#endif //__VRExampleScene_h__