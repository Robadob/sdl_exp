#ifndef __Device_h__
#define __Device_h__
#include <openvr/openvr.h>
#include <memory>
#include "../Entity2.h"
#include <glm/gtx/euler_angles.hpp>

class Device
{
    friend class TrackedDevicesVR;
public:
    Device(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model);
    virtual ~Device(){};
    virtual void update();
    void render();
    glm::mat4 getModelMat() const { return poseMat; };
    const glm::mat4 *getModelMatPtr() const { return &poseMat; };
    glm::vec3 getLocation() const { return location; }
    const glm::vec3 *getLocationPtr() const { return &location; }
    glm::vec3 getEulerAngles() const
    {
        glm::vec3 rtn; glm::extractEulerAngleXYZ(poseMat, rtn.x, rtn.y, rtn.z);
        return rtn;
    }
    vr::ETrackedDeviceClass getType() const { return type; }
    //Scene Graph FNs
    void setSceneGraphVisible(bool t){ renderSceneGraph = t; }
    void addChild(std::weak_ptr<Renderable> a, const glm::mat4 *b = nullptr, unsigned int shaderIndex = 0);
    void clearChildren();
    unsigned int removeChildren(const std::shared_ptr<Renderable> &a);
    unsigned int removeChildren(const glm::mat4 * &b);
    unsigned int removeChildren(const std::shared_ptr<Renderable> &a, const glm::mat4 * &b, const unsigned int shaderIndex = 0);
    void copySceneGraph(std::shared_ptr<Renderable> r);
    void copySceneGraph(std::shared_ptr<Device> r);
protected:
    vr::IVRSystem * const vr_HMD;
    const unsigned int id;
private:
    bool renderSceneGraph;
    void setPose(const glm::mat4 &poseMat);

    std::shared_ptr<Entity2> model;
    const vr::ETrackedDeviceClass type;
    glm::vec3 location;
    glm::mat4 poseMat;
};
class Controller : public Device
{
    /* Identifies what kind of axis is on the controller at index n. Read this type 
    * with pVRSystem->Get( nControllerDeviceIndex, Prop_Axis0Type_Int32 + n );
    */
    //enum EVRControllerAxisType
    //{
    //    k_eControllerAxis_None = 0,
    //    k_eControllerAxis_TrackPad = 1,
    //    k_eControllerAxis_Joystick = 2,
    //    k_eControllerAxis_Trigger = 3, // Analog trigger data is in the X axis
    //};
     
public:
    //Hand defaults to right if undetermined
    Controller(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model);
    enum Hand { Left, Right, Invalid };
    enum ButtonState { Free, Touched, Pressed };
    enum AxisMode { };
    Hand getHand() { return hand; };
    ButtonState getButtonState(vr::EVRButtonId id);
    glm::vec2 getAxisState(unsigned int id);
    vr::EVRControllerAxisType getAxisMode(unsigned int id);
    void update() override;
    /**
     * 1 second is 1000 milliseconds
     * Defaults to 0.5 seconds
     * @note 3999 is the upper limit for duration, its not very strong regardless
     */
    void vibrate(unsigned short durationMicroSeconds = 3999);
private:
    ButtonState buttons[vr::k_EButton_Max];
    glm::vec2 axis[vr::k_unControllerStateAxisCount];
    vr::EVRControllerAxisType axisMode[vr::k_unControllerStateAxisCount];
    Hand hand;
    uint32_t unPacketNum;
};
class Headset : public Device
{
public:
    Headset(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model);
};

#endif //__Device_h__