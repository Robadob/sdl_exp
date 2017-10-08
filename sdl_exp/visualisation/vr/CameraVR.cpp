#include "CameraVR.h"

CameraVR::CameraVR(vr::IVRSystem *vr_HMD, float nearClip, float farClip)
    : vr_HMD(vr_HMD)
    , nearClip(nearClip)
    , farClip(farClip)
    , projectionLeft(getProjectionEye(vr::Eye_Left))
    , projectionRight(getProjectionEye(vr::Eye_Right))
    , eyePosLeft(getPoseEye(vr::Eye_Left))
    , eyePosRight(getPoseEye(vr::Eye_Right))
{
    assert(vr_HMD);
}
void CameraVR::useLeft()
{
    //matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
}
void CameraVR::useRight()
{
    //matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
}
void CameraVR::update()
{
    vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

    m_iValidPoseCount = 0;
    m_strPoseClasses = "";
    for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
    {
        if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
        {
            m_iValidPoseCount++;
            m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
            if (m_rDevClassChar[nDevice] == 0)
            {
                switch (m_pHMD->GetTrackedDeviceClass(nDevice))
                {
                case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
                case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
                case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
                case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
                case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
                default:                                       m_rDevClassChar[nDevice] = '?'; break;
                }
            }
            m_strPoseClasses += m_rDevClassChar[nDevice];
        }
    }

    if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
    {
        HMDPose = glm::inverse(m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd]);
    }
}
glm::mat4 CameraVR::getProjectionEye(vr::Hmd_Eye nEye)
{
    if (!vr_HMD)
        return glm::mat4();

    vr::HmdMatrix44_t mat = vr_HMD->GetProjectionMatrix(nEye, nearClip, farClip);

    return glm::mat4(
        mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
        mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
        mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
        mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
        );
}

glm::mat4 CameraVR::getPoseEye(vr::Hmd_Eye nEye)
{
    if (!vr_HMD)
        return glm::mat4();

    vr::HmdMatrix34_t matEyeRight = vr_HMD->GetEyeToHeadTransform(nEye);
    glm::mat4 matrixObj(
        matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
        matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
        matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
        matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
        );

    return inverse(matrixObj);
}