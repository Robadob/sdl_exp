#include "HMDCamera.h"

HMDCamera::HMDCamera(vr::IVRSystem *vr_HMD, float nearClip, float farClip)
    : Camera(glm::vec3(0))
	, activeEye(Right)
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
	vr_HMD->GetRecommendedRenderTargetSize(&renderTargetDims.x, &renderTargetDims.y);
}
void HMDCamera::useLeft()
{
	if (activeEye == Left)
		return;
	viewMat = eyePosLeft * HMDPose;
	makeSkyboxViewMat();
	projMat = projectionLeft;
	activeEye = Left;
}
void HMDCamera::useRight()
{
	if (activeEye == Right)
		return;
	viewMat = eyePosRight *  HMDPose;
	makeSkyboxViewMat();
	projMat = projectionRight;
	activeEye = Right;
}
void HMDCamera::use(Eye eye)
{
	if (activeEye == eye)
		return;
	if (eye == Left)
		useLeft();
	else
		useRight();
}
void HMDCamera::makeSkyboxViewMat()
{//Strip location from viewMat (Maybe we need to only strip from HMDPose?)
	memcpy(&skyboxViewMat, &viewMat, sizeof(float) * 12);//Column major, and we only want the first 3
	//skyboxViewMat = glm::mat4(
	//	viewMat[0][0], viewMat[1][0], viewMat[2][0], 0,
	//	viewMat[0][1], viewMat[1][1], viewMat[2][1], 0,
	//	viewMat[0][2], viewMat[1][2], viewMat[2][2], 0,
	//	viewMat[0][3], viewMat[1][3], viewMat[2][3], 0
	//	);
}

const glm::uvec2& HMDCamera::getWindowDims() const
{
	return renderTargetDims;
}

void HMDCamera::setHMDPose(const glm::mat4& modelMat)
{
	HMDPose = modelMat;
	eye = HMDPose*glm::vec4(0, 0, 0, 1);
	use(activeEye);
}

glm::mat4 HMDCamera::getProjectionEye(vr::Hmd_Eye nEye)
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

glm::mat4 HMDCamera::getPoseEye(vr::Hmd_Eye nEye)
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