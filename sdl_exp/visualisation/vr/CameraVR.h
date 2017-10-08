#ifndef __CameraVR_h__
#define __CameraVR_h__
#include <glm/glm.hpp>
#include <openvr/openvr.h>

class CameraVR
{
	friend class TrackedDevicesVR;
	enum Eye{Left, Right};
public:
    CameraVR(vr::IVRSystem *vr_HMD, float nearClip, float farClip);
    //Toggles which viewproj matrix is returned
    void useLeft();
    void useRight();
	void use(Eye eye);

	/**
	* Returns a constant pointer to the cameras modelview matrix
	* This pointer can be used to continuously track the modelview matrix
	* @return A pointer to the modelview matrix
	*/
	const glm::mat4 *getVPMatPtr() const;
	/**
	* Returns a constant pointer to the cameras skybox modelview matrix
	* This pointer can be used to continuously track the skybox modelview matrix
	* @return A pointer to the modelview matrix
	*/
	const glm::mat4 *getSkyboxVPMatPtr() const;

	const glm::vec3 *getEyePtr() const { return &eye; };
	glm::vec3 getEye() const { return eye; };
private:
	Eye activeEye;
	void setHMDPose(const glm::mat4 &modelMat);
    vr::IVRSystem *vr_HMD;

    float nearClip, farClip;

    glm::mat4 getProjectionEye(vr::Hmd_Eye nEye);
    glm::mat4 getPoseEye(vr::Hmd_Eye nEye);

	glm::mat4 vpMat;

    glm::mat4 projectionLeft, projectionRight;
    glm::mat4 eyePosLeft, eyePosRight;
    glm::mat4 HMDPose;
	glm::vec3 eye;
};

#endif //__CameraVR_h__