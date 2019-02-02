#include "Device.h"

Device::Device(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model)
    : vr_HMD(vr_HMD)
    , id(id)
    //, renderSceneGraph(true)
    , model(model)//Presume id 0 == HMD
    , type(vr_HMD->GetTrackedDeviceClass(id))
{
}

void Device::update()
{

}
void Device::render(const unsigned int &shaderIndex, const glm::mat4 &transform)
{
    model->render(shaderIndex, transform);
}
//Scene Graph Forwards
//void Device::addChild(std::weak_ptr<Renderable> a, const glm::mat4 *b, unsigned int shaderIndex)
//{
//    model->addChild(a, b, shaderIndex);
//}
//void Device::clearChildren()
//{
//    model->clearChildren();
//}
//unsigned int Device::removeChildren(const std::shared_ptr<Renderable> &a)
//{
//    return model->removeChildren(a);
//}
//unsigned int Device::removeChildren(const glm::mat4 * &b)
//{
//    return model->removeChildren(b);
//}
//unsigned int Device::removeChildren(const std::shared_ptr<Renderable> &a, const glm::mat4 * &b, const unsigned int shaderIndex)
//{
//    return model->removeChildren(a, b, shaderIndex);
//}
//void Device::copySceneGraph(std::shared_ptr<Renderable> r)
//{
//    model->copySceneGraph(r);
//}
//void Device::copySceneGraph(std::shared_ptr<Device> r)
//{
//    model->copySceneGraph(r->model);
//}

void Device::setPose(const glm::mat4 &poseMat)
{
    model->setSceneMat(poseMat); 
    location = glm::vec3(poseMat * glm::vec4(0, 0, 0, 1));
};

Controller::Controller(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model)
    : Device(vr_HMD, id, model)
    , unPacketNum(0)
{
    switch (vr_HMD->GetControllerRoleForTrackedDeviceIndex(id))
    {
    case vr::ETrackedControllerRole::TrackedControllerRole_LeftHand:
        hand = Left;
        break;
    case vr::ETrackedControllerRole::TrackedControllerRole_RightHand:
        hand = Right;
        break;
    case vr::ETrackedControllerRole::TrackedControllerRole_Invalid:
    default:
        hand = Invalid;
        break;
    }
}
void Controller::update()
{
    vr::VRControllerState_t _state;
    if (vr_HMD->GetControllerState(id, &_state, sizeof(_state)))
    {//https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetControllerState
        if (unPacketNum != _state.unPacketNum)
        {//State has changed
            unPacketNum = _state.unPacketNum;
            //Process Buttons
            for (unsigned int i = 0; i < vr::k_EButton_Max;++i)
            {
                unsigned long long id = 1 << i;
                buttons[i] = (id&_state.ulButtonPressed) ? Pressed : ((id&_state.ulButtonTouched)?Touched:Free);
            }
            //Process Axis
            for (unsigned int i = 0; i < vr::k_unControllerStateAxisCount; ++i)
            {
                axis[i] = glm::vec2(_state.rAxis[i].x, _state.rAxis[i].y);
            }
        }
    }
}
Controller::ButtonState Controller::getButtonState(vr::EVRButtonId id)
{
    return buttons[id];
}
glm::vec2 Controller::getAxisState(unsigned int id)
{
    return axis[id];
}
vr::EVRControllerAxisType Controller::getAxisMode(unsigned int id)
{
    return axisMode[id];
}

Headset::Headset(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model)
    : Device(vr_HMD, id, model)
{

}
void Controller::vibrate(unsigned short duration)
{
    assert(duration < 4000);
    vr_HMD->TriggerHapticPulse(id, 0, duration);
}