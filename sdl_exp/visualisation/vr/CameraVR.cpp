#include "CameraVR.h"

CameraVR::CameraVR(vr::IVRSystem *vr_HMD, float nearClip, float farClip)
    : activeEye(Right)
    , vr_HMD(vr_HMD)
    , nearClip(nearClip)
    , farClip(farClip)
    , projectionLeft(getProjectionEye(vr::Eye_Left))
    , projectionRight(getProjectionEye(vr::Eye_Right))
    , eyePosLeft(getPoseEye(vr::Eye_Left))
	, eyePosRight(getPoseEye(vr::Eye_Right))
{
    assert(vr_HMD);
	useLeft();
}
void CameraVR::useLeft()
{
	if (activeEye == Left)
		return;
	vpMat = projectionLeft * eyePosLeft * HMDPose;
	activeEye = Left;
}
void CameraVR::useRight()
{
	if (activeEye == Right)
		return;
	vpMat = projectionRight * eyePosRight *  HMDPose;
	activeEye = Right;
}
void CameraVR::use(Eye eye)
{
	if (activeEye == eye)
		return;
	if (eye == Left)
		useLeft();
	else
		useRight();
}

const glm::mat4* CameraVR::getVPMatPtr() const
{
	return &vpMat;
}

const glm::mat4* CameraVR::getSkyboxVPMatPtr() const
{
	return &vpMat;//Purge translation from this
}

void CameraVR::setHMDPose(const glm::mat4& modelMat)
{
	HMDPose = modelMat;
	eye = HMDPose*glm::vec4(0, 0, 0, 1);
	use(activeEye);
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