#include "SceneVR.h"
#include "VisualisationVR.h"

//SceneVR::SceneVR(VisualisationVR &vis) : visVR(&vis){ };
//Scene(vis), 
SceneVR::SceneVR(VisualisationVR &vis)
    :visVR(vis)
{
    vis.setVR(this);
}
std::shared_ptr<TrackedDevicesVR> SceneVR::getTrackedDevices(){ return visVR.vr_renderModels; }