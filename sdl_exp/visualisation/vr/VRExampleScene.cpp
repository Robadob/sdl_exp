#include "VRExampleScene.h"
#include "../VisualisationVR.h"
#include <glm/gtx/transform.hpp>
#define lanceLength 0.25
std::shared_ptr<Controller> leftCtrller, rightCtrller;
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
	auto rc = getTrackedDevices()->getRightController();
	if (rc)
		rc->addChild(lines);
    lines->addChild(this->points);
    leftCtrller = getTrackedDevices()->getLeftController();
    rightCtrller = getTrackedDevices()->getRightController();
}

void VRExampleScene::render()
{
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        if (i != heldSphere)
            this->sphere[i]->render();
    }
}
void VRExampleScene::update(unsigned int frameTime)
{
    //getTrackedDevices()->getCamera()->setWorldMat(glm::translate(wMat, glm::vec3(0.001, 0, 0)));
    //getTrackedDevices()->getCamera()->setWorldMat();
    //if (!leftCtrller)
    //    leftCtrller = getTrackedDevices()->getLeftController();
    //if (!rightCtrller)
    //    rightCtrller = getTrackedDevices()->getRightController();
    //if (leftCtrller&&rightCtrller)
    //{
    //    //If both grip buttons are pressed, we want to transform the world (scale/translate)
    //    static bool prevGripLeftDown = false;
    //    static bool prevGripRightDown = false;
    //    static glm::vec2 prevControllerLeftPos;
    //    static glm::vec2 prevControllerRightPos;
    //    bool gripLeftDown = leftCtrller->getButtonState(vr::k_EButton_Grip) == Controller::ButtonState::Pressed;
    //    bool gripRightDown = rightCtrller->getButtonState(vr::k_EButton_Grip) == Controller::ButtonState::Pressed;
    //    glm::vec2 controllerLeftPos = glm::vec2(leftCtrller->getLocation().x, leftCtrller->getLocation().z);
    //    glm::vec2 controllerRightPos = glm::vec2(rightCtrller->getLocation().x, rightCtrller->getLocation().z);
    //    if (prevGripLeftDown&&prevGripRightDown&&gripLeftDown&&gripRightDown)
    //    {//2 frames with both down
    //        const glm::mat4 _wMat = getTrackedDevices()->getCamera()->getWorldMat();
    //        glm::mat4 wMat = _wMat;
    //        //{//SCALE
    //        //    float prevDistance = glm::distance(prevControllerLeftPos, prevControllerRightPos);
    //        //    float currentDistance = glm::distance(controllerLeftPos, controllerRightPos);
    //        //    float invScaleFactor = prevDistance / currentDistance;
    //        //    wMat = glm::scale(wMat, glm::vec3(invScaleFactor));
    //        //}
    //        {//UN-TRANSLATE
    //            wMat = glm::translate(wMat, glm::vec3(controllerLeftPos.x, 0, controllerLeftPos.y));
    //        }
    //        {//ROTATE
    //            glm::vec2 prevAngle = glm::normalize(prevControllerLeftPos - prevControllerRightPos);
    //            glm::vec2 currentAngle = glm::normalize(controllerLeftPos - controllerRightPos);
    //            float invAngle = atan2(currentAngle.y, currentAngle.x) - atan2(prevAngle.y, prevAngle.x);
    //            wMat = glm::rotate(wMat, invAngle, glm::vec3(0, 1, 0));
    //        }
    //        {//TRANSLATE
    //            wMat = glm::translate(wMat, -glm::vec3(prevControllerLeftPos.x, 0, prevControllerLeftPos.y));
    //            //glm::vec3 newLeftControllerLoc = inverse(_wMat)*wMat*glm::vec4(controllerLeftPos.x, 0, controllerLeftPos.y, 1.0f);
    //            //glm::vec2 newLeftControllerLoc2 = glm::vec2(newLeftControllerLoc.x, newLeftControllerLoc.z);
    //            //glm::vec2 invOffset = prevControllerLeftPos - newLeftControllerLoc2;
    //            //wMat = glm::translate(wMat, glm::vec3(invOffset.x, 0, invOffset.y));
    //        }
    //        //Stabilise y==0
    //        ///At current unnecessary
    //        //Scale near/far planes?
    //        ///CBA right now

    //        getTrackedDevices()->getCamera()->setWorldMat(wMat);
    //    }
    //    //Update previous values
    //    {
    //        prevGripLeftDown = gripLeftDown;
    //        prevGripRightDown = gripRightDown;
    //        prevControllerLeftPos = controllerLeftPos;
    //        prevControllerRightPos = controllerRightPos;
    //    }

    //}
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
    else if (vr::k_EButton_Grip == buttonId&&vr::VREvent_ButtonTouch == buttonEvent)
    {
        controller->vibrate();
    }
    else if (vr::k_EButton_SteamVR_Touchpad == buttonId&&vr::VREvent_ButtonTouch == buttonEvent)
    {
        controller->vibrate();
    }
    return false;
};