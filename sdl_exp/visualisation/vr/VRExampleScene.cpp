#include "VRExampleScene.h"
#include "../VisualisationVR.h"

VRExampleScene::VRExampleScene(VisualisationVR &vis)
    : BasicScene(vis.toViewportExt())
    , SceneVR(vis)
{
    srand((unsigned int)time(0));
    const glm::vec3 *hmdLoc = getTrackedDevices()->getHMD()->getLocationPtr();
    for (int i = 0; i < MAX_SPHERES;++i)
    {
        this->sphere[i] = std::make_shared<Entity>(Stock::Models::SPHERE, (float)SPHERE_RAD*2, Stock::Shaders::PHONG);
        this->sphere[i]->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
        this->sphere[i]->setLocation(glm::vec3(
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10)
            ));
        this->sphere[i]->getShaders()->addDynamicUniform("_lightSource", (GLfloat*)hmdLoc, 3);
        registerEntity(this->sphere[i]);
    }
    this->setSkybox(true);
    this->visualisation.setWindowTitle("VR Example Companion");
    this->setRenderAxis(true);
}

void VRExampleScene::render()
{
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        this->sphere[i]->render();
    }
}
bool VRExampleScene::controllerEventVR(std::shared_ptr<Controller> controller, vr::EVRButtonId buttonId, vr::EVREventType buttonEvent)
{
    if (
        Controller::Right  == controller->getHand() &&
        vr::k_EButton_SteamVR_Trigger == buttonId
        )
    {
        if (vr::VREvent_ButtonPress == buttonEvent)
        {
            for (int i = 0; i < MAX_SPHERES; ++i)
            {
                if (glm::distance(sphere[i]->getLocation(), controller->getLocation())<SPHERE_RAD)
                {//Controller touching sphere
                    //Refresh colour
                    this->sphere[i]->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
                }
            }
        }
        else if (vr::VREvent_ButtonUnpress == buttonEvent)
        {
            
        }
    }
    return false;
};