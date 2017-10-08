#ifndef __TrackedDevicesVR_h__
#define __TrackedDevicesVR_h__

#include <vector>
#include <openvr/openvr.h>
#include <memory>
#include "../Entity2.h"
#include "glm/gtx/euler_angles.hpp"
#include "CameraVR.h"

/**
 * Holds the models used to render controllers etc
 */
class TrackedDevicesVR
{
	class Device
	{
	public:
		Device(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model);
		void update();
		void render();
		void setVisible(bool visible){ this->visible = visible; }
		void setPose(const glm::mat4 &poseMat){ this->poseMat = id == vr::k_unTrackedDeviceIndex_Hmd ? inverse(poseMat) : poseMat; };
		glm::mat4 getModelMat() const { return poseMat; };
		glm::vec4 getLocation() const { return poseMat * glm::vec4(0, 0, 0, 1); }
		//glm::vec3 getScale(){ //glm::decompose(poseMat,,,,) }
		glm::vec3 getEulerAngles() const
		{
			glm::vec3 rtn; glm::extractEulerAngleXYZ(poseMat, rtn.x, rtn.y, rtn.z);
			return rtn;
		}
	private:
		uint32_t unPacketNum;
		vr::IVRSystem *vr_HMD;
		const unsigned int id;
		std::shared_ptr<Entity2> model;
		vr::ETrackedDeviceClass type;
		bool visible;
		glm::mat4 poseMat;
	};
public:
    TrackedDevicesVR(vr::IVRSystem *vr_HMD);
    ~TrackedDevicesVR();
    bool getInitState(){ return initState; }
	void update();
	void render();
	std::shared_ptr<Device> getHMD(){ return trackedDevices[vr::k_unTrackedDeviceIndex_Hmd]; }
	std::shared_ptr<CameraVR> getCamera() const { return camera; }
private:
	void updatePoses();
	bool addDevice(unsigned int id);
	bool removeDevice(unsigned int id);
    std::shared_ptr<Entity2> findLoadRenderModel(std::string &modelName);
    bool initState;
    vr::IVRSystem *HMD;                 //HMD configuration access
    vr::IVRRenderModels *renderModels;   //Hardware rendermodels access
    std::map<std::string, std::shared_ptr<Entity2>> renderEntities;
	std::shared_ptr<Device> trackedDevices[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
	std::shared_ptr<CameraVR> camera;
};

#endif //__TrackedDevicesVR_h__
