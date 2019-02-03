#include "VRExampleScene.h"
#include "VisualisationVR.h"
#include <glm/gtx/transform.hpp>
#define lanceLength 0.25
std::shared_ptr<Controller> leftCtrller, rightCtrller;
VRExampleScene::VRExampleScene(VisualisationVR &vis)
    : BasicScene(vis.toViewportExt())
    , SceneVR(vis)
    , heldSphere(UINT_MAX)
    , pen(std::make_shared<Draw>())
{
    srand((unsigned int)time(0));
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        this->sphere[i] = std::make_shared<Entity>(Stock::Models::SPHERE, (float)SPHERE_RAD * 2, Stock::Shaders::PHONG);
        glm::vec3 color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
        this->sphere[i]->setMaterial(color/10.0f, color);
        this->sphere[i]->setSceneMat(glm::translate(glm::mat4(),glm::vec3(
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20),
            ((rand() / (float)RAND_MAX) * SPHERE_RAD * 20) - (SPHERE_RAD * 10)
            )));
        registerEntity(this->sphere[i]);
    }
    {
        pen->begin(Draw::Lines, "lance_line");
        pen->width(1.0f);
        pen->color(glm::vec4(1, 1, 1, 0.3));
        pen->vertex(glm::vec3(0));
        pen->vertex(glm::vec3(0, 0, -lanceLength));
        pen->save();
        lance = pen->makePortable("lance_line");
    }
    {
        pen->begin(Draw::Points, "lance_pointer");
        pen->width(5.0f);
        pen->color(glm::vec4(1, 0, 0, 0.3));
        pen->vertex(glm::vec3(0, 0, -lanceLength));
        pen->save();
        pointer = pen->makePortable("lance_pointer");
    }

    registerEntity(this->pen);
    this->setSkybox(true);
    this->setRenderAxis(true);
    this->visualisation.setWindowTitle("VR Example Companion");
	auto rc = getTrackedDevices()->getRightController();
	if (rc)
    {
        rc->attach(lance, "lance");
        rc->attach(pointer, "pointer");
	}
    leftCtrller = getTrackedDevices()->getLeftController();
    rightCtrller = getTrackedDevices()->getRightController();
    //Init Lighting
    {
        PointLight _p = Lights()->addPointLight();
        _p.Ambient(glm::vec3(0.0f));
        _p.Diffuse(glm::vec3(0.5f));
        _p.Specular(glm::vec3(0.02f));
        _p.ConstantAttenuation(0.5f);
    }
}

void VRExampleScene::render()
{
    for (int i = 0; i < MAX_SPHERES; ++i)
    {
        if (i != heldSphere)
            this->sphere[i]->render();
    }
}
void VRExampleScene::update(const unsigned int &frameTime)
{
    //Emulate full bright, attach the one light source to the camera
    {
        const glm::vec3 *hmdLoc = getTrackedDevices()->getHMD()->getLocationPtr();
        auto p = Lights()->getPointLight(0);
        p.Position(*hmdLoc);
    }
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
    //Controller reconnected, rebuild scene graph
    if (Controller::Right == controller->getHand() && buttonEvent == vr::VREvent_TrackedDeviceActivated)
    {
        controller->attach(lance, "lance");
        controller->attach(pointer, "pointer");
        rightCtrller = controller;
    }
    if (Controller::Left == controller->getHand() && buttonEvent == vr::VREvent_TrackedDeviceActivated)
    {
        leftCtrller = controller;
    }
    if (Controller::Right == controller->getHand() && buttonEvent == vr::VREvent_TrackedDeviceDeactivated)
    {
        controllerEventVR(controller, vr::k_EButton_SteamVR_Trigger, vr::VREvent_ButtonUnpress);//Simulate dropping item
        rightCtrller.reset();
    }
    if (Controller::Left == controller->getHand() && buttonEvent == vr::VREvent_TrackedDeviceDeactivated)
    {
        leftCtrller.reset();
    }
    if (
        Controller::Right  == controller->getHand() &&
        vr::k_EButton_SteamVR_Trigger == buttonId
        )
    {
        if (vr::VREvent_ButtonPress == buttonEvent)
        {
            if (heldSphere==UINT_MAX)
            {
                glm::vec3 lancePoint = controller->getModelMat() * glm::vec4(0, 0, -lanceLength, 1.0f);
                for (int i = 0; i < MAX_SPHERES; ++i)
                {
                    glm::vec3 sphereLoc = sphere[i]->getLocation();
                    if (glm::distance(sphereLoc, lancePoint)<SPHERE_RAD)
                    {//Controller touching sphere
                        heldSphere = i;
                        //Calculate sphere model mat
                        //glm::vec3 sphereOffset = glm::vec3(sphere[i]->getModelMat()*glm::vec4(0, 0, 0, 1)) - controller->getLocation();
                        heldSphereMat4 = glm::inverse(controller->getSceneMat());
                        //sphere[i]->setLocation(glm::vec3(0));
                        controller->attach(sphere[i], "held_item", heldSphereMat4);
                        //Refresh colour
                        glm::vec3 color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
                        this->sphere[i]->setMaterial(color / 10.0f, color);
                        break;
                    }
                }
                {//Should just make a convenient way to edit drawing
                    pen->begin(Draw::Points, "lance_pointer");
                    pen->width(5.0f);
                    pen->color(glm::vec4(0, 1, 0, 0.3));
                    pen->vertex(glm::vec3(0, 0, -lanceLength));
                    pen->save(true);
                }
            }
        }
        else if (vr::VREvent_ButtonUnpress == buttonEvent)
        {
            //Drop any held spheres
            if (heldSphere != UINT_MAX)
            {
                controller->detach("held_item");
                sphere[heldSphere]->setSceneMat(controller->getSceneMat()*heldSphereMat4*sphere[heldSphere]->getSceneMat());
                heldSphere = UINT_MAX;
            }
            {//Should just make a convenient way to edit drawing
                pen->begin(Draw::Points, "lance_pointer");
                pen->width(5.0f);
                pen->color(glm::vec4(1, 0, 0, 0.3));
                pen->vertex(glm::vec3(0, 0, -lanceLength));
                pen->save(true);
            }
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