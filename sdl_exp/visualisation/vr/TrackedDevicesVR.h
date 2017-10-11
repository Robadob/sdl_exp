#ifndef __TrackedDevicesVR_h__
#define __TrackedDevicesVR_h__

#include <vector>
#include <openvr/openvr.h>
#include <memory>
#include "../Entity2.h"
#include "glm/gtx/euler_angles.hpp"
#include "HMDCamera.h"

/**
 * Holds the models used to render controllers etc
 */
class TrackedDevicesVR
{
	class Device
	{
	public:
		Device(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model);
		virtual ~Device(){};
		void update();
		void render();
		void setPose(const glm::mat4 &poseMat){ this->poseMat = id == vr::k_unTrackedDeviceIndex_Hmd ? inverse(poseMat) : poseMat; };
		glm::mat4 getModelMat() const { return poseMat; };
		glm::vec4 getLocation() const { return poseMat * glm::vec4(0, 0, 0, 1); }
		glm::vec3 getEulerAngles() const
		{
			glm::vec3 rtn; glm::extractEulerAngleXYZ(poseMat, rtn.x, rtn.y, rtn.z);
			return rtn;
		}
		vr::ETrackedDeviceClass getType() const { return type; }
	private:
		uint32_t unPacketNum;
		vr::IVRSystem *vr_HMD;
		const unsigned int id;
		std::shared_ptr<Entity2> model;
		vr::ETrackedDeviceClass type;
		glm::mat4 poseMat;
	};
	class Controller : public Device
	{
	public:
		//Hand defaults to right if undetermined
		Controller(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model)
			: Device(vr_HMD, id, model)
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
		enum Hand { Left, Right, Invalid };
		Hand getHand();
	private:
		Hand hand;
	};
public:
    TrackedDevicesVR(vr::IVRSystem *vr_HMD);
    ~TrackedDevicesVR();
    bool getInitState(){ return initState; }
	void update();
	void render();
	std::shared_ptr<Device> getHMD(){ return trackedDevices[vr::k_unTrackedDeviceIndex_Hmd]; }
	std::shared_ptr<HMDCamera> getCamera() const { return camera; }
	void setLighthouseVisible(bool t){ renderLighthouses = t; };
	void setControllersVisible(bool t){ renderLeftController = t; renderRightController = t; renderInvalidController = t; };
	void setLeftControllerVisible(bool t){ renderLeftController = t; };
	void setRightControllerVisible(bool t){ renderRightController = t; };
	void setInvalidControllerVisible(bool t){ renderInvalidController = t; };
	void setGenericTrackerVisible(bool t){ renderTracker = t; };
	void setHeadsetVisible(bool t){ renderHeadset = t; };
	void setUnknownDeviceVisible(bool t){ renderUnknown = t; }
private:
	bool renderLighthouses, renderRightController, renderLeftController, renderInvalidController, renderTracker, renderHeadset, renderUnknown;
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
	std::shared_ptr<HMDCamera> camera;
};

#endif //__TrackedDevicesVR_h__
