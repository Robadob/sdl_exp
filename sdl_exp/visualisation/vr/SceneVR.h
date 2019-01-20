#ifndef __SceneVR_h__
#define __SceneVR_h__

#include "../interface/Scene.h"
#include "Device.h"
#include "HMDCamera.h"

class VisualisationVR;

class SceneVR//: public Scene
{
public:
    /**
     * Called when the user presses a button on a tracked controller
     * @param controller The affected controller
     * @param buttonId The affected button
     * @param buttonEvent The event triggered
     * @return Returning true permits other objects to handle the event
     */
    virtual bool controllerEventVR(std::shared_ptr<Controller> controller, vr::EVRButtonId buttonId, vr::EVREventType buttonEvent) { return true; };//button, newState
protected:

    std::shared_ptr<TrackedDevicesVR> getTrackedDevices();

    // SceneVR(VisualisationVR &vis);
    SceneVR(VisualisationVR &vis);
    /**
     * Stores a copy that can be used to access the VR specifics
     * 
     */
private:
   VisualisationVR &visVR;
};

#endif //__SceneVR_h__
