#ifndef __CameraVR_h__
#define __CameraVR_h__

#include <openvr/openvr.h>

#include "../interface/Camera.h"
#include "../interface/Viewport.h"

/**
 * Tracks and provides the View and Projection matricies for a camera tracking an OpenVR Head Mounted Display
 */
class HMDCamera : public Camera, public Viewport
{
	friend class TrackedDevicesVR;
	enum Eye{Left, Right};
public:
    HMDCamera(vr::IVRSystem *vr_HMD, float nearClip, float farClip);
    //Toggles which viewproj matrix is returned
    void useLeft();
    void useRight();
	void use(Eye eye);
	const glm::mat4 *getProjMatPtr()const override { return &projMat; };
	const glm::mat4 getProjMat()const override { return projMat; };
	/**
	* @return The current viewport dimensions
	*/
	const glm::uvec2& getWindowDims() const override;
private:
	Eye activeEye;
	void setHMDPose(const glm::mat4 &modelMat);
	void makeSkyboxViewMat();
    vr::IVRSystem *vr_HMD;

    float nearClip, farClip;

	glm::uvec2 renderTargetDims;

    glm::mat4 getProjectionEye(vr::Hmd_Eye nEye);
    glm::mat4 getPoseEye(vr::Hmd_Eye nEye);

    glm::mat4 projectionLeft, projectionRight;
    glm::mat4 eyePosLeft, eyePosRight;
	glm::mat4 HMDPose;

	glm::mat4 projMat;
};

#endif //__CameraVR_h__