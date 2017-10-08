#ifndef __RenderModelsVR_h__
#define __RenderModelsVR_h__

#include <vector>
#include <openvr/openvr.h>
#include <memory>
#include "../Entity2.h"

/**
 * Holds the models used to render controllers etc
 */
class RenderModelsVR
{
public:
    RenderModelsVR(vr::IVRSystem *vr_HMD);
    ~RenderModelsVR();
    bool getInitState(){ return initState; }
    bool addDevice(unsigned int id);
    bool removeDevice(unsigned int id);
private:
    std::shared_ptr<Entity2> findLoadRenderModel(std::string &modelName);
    bool initState;
    vr::IVRSystem *HMD;                 //HMD configuration access
    vr::IVRRenderModels *renderModels;   //Hardware rendermodels access
    std::map<std::string, std::shared_ptr<Entity2>> renderEntities;
    std::shared_ptr<Entity2> trackedDeviceToRenderEntity[vr::k_unMaxTrackedDeviceCount];
    bool showTrackedDevice[vr::k_unMaxTrackedDeviceCount];
};

#endif //__RenderModelsVR_h__