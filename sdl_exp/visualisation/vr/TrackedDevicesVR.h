#ifndef __TrackedDevicesVR_h__
#define __TrackedDevicesVR_h__

#include <vector>
#include <openvr/openvr.h>
#include <memory>
#include "../Entity2.h"
#include "glm/gtx/euler_angles.hpp"
#include "HMDCamera.h"
#include "Device.h"
#include "SceneVR.h"

/**
 * Holds the models used to render controllers etc
 */
class TrackedDevicesVR
{
	
public:
    TrackedDevicesVR(vr::IVRSystem *vr_HMD);
    ~TrackedDevicesVR();
    bool getInitState(){ return initState; }
    void update(SceneVR *scene);
    void render();
    std::shared_ptr<HMDCamera> getCamera() const { return camera; }
    std::shared_ptr<Headset> getHMD(){ return std::dynamic_pointer_cast<Headset>(trackedDevices[vr::k_unTrackedDeviceIndex_Hmd]); }
    std::shared_ptr<Controller> getController(const unsigned int &no);
    std::shared_ptr<Controller> getLeftController();
    std::shared_ptr<Controller> getRightController();
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
