#ifndef __CameraVR_h__
#define __CameraVR_h__
#include <glm/glm.hpp>
#include <openvr/openvr.h>

class CameraVR
{
public:
    CameraVR(vr::IVRSystem *vr_HMD, float nearClip, float farClip);
    //Toggles which viewproj matrix is returned
    void useLeft();
    void useRight();
    void update();
private:
    vr::IVRSystem *vr_HMD;

    float nearClip, farClip;

    glm::mat4 getProjectionEye(vr::Hmd_Eye nEye);
    glm::mat4 getPoseEye(vr::Hmd_Eye nEye);

    glm::mat4 projectionLeft, projectionRight;
    glm::mat4 eyePosLeft, eyePosRight;
    glm::mat4 HMDPose;
};

#endif //__CameraVR_h__