#include "VRExampleScene.h"
#include "../VisualisationVR.h"
#include <glm/gtx/transform.hpp>
#define lanceLength 0.25
VRExampleScene::VRExampleScene(VisualisationVR &vis)
    : BasicScene(vis.toViewportExt())
    , SceneVR(vis)
    , heldSphere(UINT_MAX)
{
    srand((unsigned int)time(0));
    const glm::vec3 *hmdLoc = getTrackedDevices()->getHMD()->getLocationPtr();
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        this->sphere[i] = std::make_shared<Entity>(Stock::Models::SPHERE, (float)SPHERE_RAD * 2, Stock::Shaders::PHONG);
        this->sphere[i]->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
        this->sphere[i]->setModelMat(glm::translate(glm::mat4(),glm::vec3(
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10)
            )));
        this->sphere[i]->getShaders()->addDynamicUniform("_lightSource", (GLfloat*)hmdLoc, 3);
        registerEntity(this->sphere[i]);
    }
    this->points = std::shared_ptr<Points>(new Points(Point(glm::vec3(0, 0, -lanceLength), glm::vec4(1, 0, 0, 0.3)), 5.0f));

    this->lines = std::make_shared<Lines>(glm::vec3(0), glm::vec3(0, 0, -lanceLength), glm::vec4(1, 1, 1, 0.3));
    registerEntity(this->points);
    registerEntity(this->lines);
    this->setSkybox(true);
    this->visualisation.setWindowTitle("VR Example Companion");
    this->setRenderAxis(true);
    getTrackedDevices()->getRightController()->addChild(lines);
    lines->addChild(this->points);
}

void VRExampleScene::render()
{
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        if (i != heldSphere)
            this->sphere[i]->render();
    }
    this->points->render();
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
            if (heldSphere==UINT_MAX)
            {
                glm::vec3 lancePoint = controller->getModelMat() * glm::vec4(points->getLocation(), 1.0f);
                for (int i = 0; i < MAX_SPHERES; ++i)
                {
                    glm::vec3 sphereLoc = sphere[i]->getModelMat() * glm::vec4(sphere[i]->getLocation(), 1.0f);
                    if (glm::distance(sphereLoc, lancePoint)<SPHERE_RAD)
                    {//Controller touching sphere
                        heldSphere = i;
                        //Calculate sphere model mat
                        //glm::vec3 sphereOffset = glm::vec3(sphere[i]->getModelMat()*glm::vec4(0, 0, 0, 1)) - controller->getLocation();
                        heldSphereMat4 = glm::inverse(controller->getModelMat());
                        //sphere[i]->setLocation(glm::vec3(0));
                        controller->addChild(sphere[i], &heldSphereMat4);
                        //Refresh colour
                        this->sphere[i]->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
                        break;
                    }
                }
                this->points->setPointColor(0, glm::vec4(0, 1, 0, 0.3));
            }
        }
        else if (vr::VREvent_ButtonUnpress == buttonEvent)
        {
            //Drop any held spheres
            if (heldSphere != UINT_MAX)
            {
                controller->removeChildren(sphere[heldSphere]);

                sphere[heldSphere]->setModelMat(controller->getModelMat()*heldSphereMat4*sphere[heldSphere]->getModelMat());
                heldSphere = UINT_MAX;
            }
            this->points->setPointColor(0, glm::vec4(1, 0, 0, 0.3));
        }
    }
    return false;
};