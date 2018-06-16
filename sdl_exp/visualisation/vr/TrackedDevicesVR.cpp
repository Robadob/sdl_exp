#include "TrackedDevicesVR.h"
#include "utilVR.h"
#include <chrono>
#include <thread>

#define NEAR_CLIP 0.005f
#define FAR_CLIP 500.0f

TrackedDevicesVR::TrackedDevicesVR(vr::IVRSystem *vr_HMD)
    : renderLighthouses(true)
    , renderRightController(true)
	, renderLeftController(true)
	, renderInvalidController(true)
	, renderTracker(true)
	, renderHeadset(false)
	, renderUnknown(true)
	, initState(true)
	, HMD(vr_HMD)
	, camera(std::make_shared<HMDCamera>(vr_HMD, NEAR_CLIP, FAR_CLIP))
{
    assert(HMD);
    vr::EVRInitError eError = vr::VRInitError_None;
    //Load render model access interface (may fail)
    renderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    if (!renderModels)
    {
        printf("Unable to get render model interface: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        initState = false;
        return;
    }

    //Setup render models
    for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
    {
        addDevice(unTrackedDevice);
    }
}

TrackedDevicesVR::~TrackedDevicesVR()
{
    for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
		trackedDevices[i].reset();
    renderEntities.clear();
}

void TrackedDevicesVR::update(SceneVR *scene)
{
	//Buffered controller states?
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (trackedDevices[unDevice])
			trackedDevices[unDevice]->update();
	}
	//OpenVR events
	vr::VREvent_t event;
	while (HMD->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{
			case vr::VREvent_TrackedDeviceUpdated:
			{
				printf("VR device %u updated.\n", event.trackedDeviceIndex);
				break;
			}
			case vr::VREvent_TrackedDeviceActivated:
			{
				addDevice(event.trackedDeviceIndex);
				printf("VR device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
				break;
			}
			case vr::VREvent_TrackedDeviceDeactivated:
			{
				removeDevice(event.trackedDeviceIndex);
				printf("VR device %u detatched. Disabling render model.\n", event.trackedDeviceIndex);
				break;
			}
            case vr::VREvent_ButtonPress:
            case vr::VREvent_ButtonUnpress:
            case vr::VREvent_ButtonTouch:
            case vr::VREvent_ButtonUntouch:
			{//Button has been event'd
                if (auto d = std::dynamic_pointer_cast<Controller>(trackedDevices[event.trackedDeviceIndex]))
                {
                    if (!scene->controllerEventVR(d, static_cast<vr::EVRButtonId>(event.data.controller.button), static_cast<vr::EVREventType>(event.eventType)))
                        break;//Break if scene denies further handling of event
                }
                else
                    fprintf(stderr, "Error: Button event recieved from device not tracked as controller.\n");
                break;                
            }
			case vr::VREvent_MouseMove:
			{//Simulated mouse has moved (does this mean controller?)
				//event.data.mouse.button == vr::VRMouseButton_Left, vr::VRMouseButton_Right, vr::VRMouseButton_Middle
               // printf("Mouse move?\n");
                //event.data.touchPadMove   
			   // printf("Touchpad move Finger:%d, first(%.3f, %.3f), raw(%.3f, %.3f), time %.3f\n", event.data.touchPadMove.bFingerDown, event.data.touchPadMove.fValueXFirst, event.data.touchPadMove.fValueYFirst, event.data.touchPadMove.fValueXRaw, event.data.touchPadMove.fValueYRaw, event.data.touchPadMove.flSecondsFingerDown);

                //Vector2 touch = vr::SteamVR_Controller HMD->ControllerInput(rightDevice).GetAxis(Valve.VR.EVRButtonId.k_EButton_Axis0);
                break;
			}
            case vr::VREvent_TouchPadMove:
            {
              //  printf("Touchpad move Finger:%d, first(%.3f, %.3f), raw(%.3f, %.3f), time %.3f\n", event.data.touchPadMove.bFingerDown, event.data.touchPadMove.fValueXFirst, event.data.touchPadMove.fValueYFirst, event.data.touchPadMove.fValueXRaw, event.data.touchPadMove.fValueYRaw, event.data.touchPadMove.flSecondsFingerDown);

                break;
            }
			case vr::VREvent_MouseButtonDown :
			//Simulated mouse down
			case vr::VREvent_MouseButtonUp :
			//Simulated mouse up
			case vr::VREvent_InputFocusCaptured :
			case vr::VREvent_InputFocusReleased :
			case vr::VREvent_None:
				//Invalid event
			default:
				break;
		}
	}
}
bool TrackedDevicesVR::addDevice(unsigned int deviceIndex)
{
    if (!HMD->IsTrackedDeviceConnected(deviceIndex))
        return false;
    if (deviceIndex >= vr::k_unMaxTrackedDeviceCount)
        return false;
    std::string sRenderModelName = vr::getDeviceString(HMD, deviceIndex, vr::Prop_RenderModelName_String);
    std::shared_ptr<Entity2> renderModel = findLoadRenderModel(sRenderModelName);
    if (!renderModel)
    {
        std::string sTrackingSystemName = vr::getDeviceString(HMD, deviceIndex, vr::Prop_TrackingSystemName_String);
        printf("Unable to load render model for tracked device %d (%s.%s)\n", deviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
		trackedDevices[deviceIndex].reset();
		return false;
    }
	switch (HMD->GetTrackedDeviceClass(deviceIndex))
	{
	case vr::TrackedDeviceClass_Controller:
		trackedDevices[deviceIndex] = std::make_shared<Controller>(HMD, deviceIndex, renderModel);
        break;
    case vr::TrackedDeviceClass_HMD:
        trackedDevices[deviceIndex] = std::make_shared<Headset>(HMD, deviceIndex, renderModel);
        break;
	case vr::TrackedDeviceClass_GenericTracker:
	case vr::TrackedDeviceClass_TrackingReference:
	default:
		trackedDevices[deviceIndex] = std::make_shared<Device>(HMD, deviceIndex, renderModel);
		break;

	}
	return true;
}
bool TrackedDevicesVR::removeDevice(unsigned int deviceIndex)
{
    if (deviceIndex >= vr::k_unMaxTrackedDeviceCount)
        return false;
	bool rtn = (bool)trackedDevices[deviceIndex];
	trackedDevices[deviceIndex].reset();
    return rtn;
}
std::shared_ptr<Entity2> TrackedDevicesVR::findLoadRenderModel(std::string &modelName)
{//Based on OpenVR sample CMainApplication::FindOrLoadRenderModel()
    if (renderEntities.find(modelName) == renderEntities.end())
    {
        //Load fresh model, existing not found
        vr::RenderModel_t *pModel;
        vr::EVRRenderModelError error;
        while (true)
        {
            error = vr::VRRenderModels()->LoadRenderModel_Async(modelName.c_str(), &pModel);
            if (error != vr::VRRenderModelError_Loading)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (error != vr::VRRenderModelError_None)
        {
            printf("Unable to load render model %s - %s\n", modelName.c_str(), vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
            return nullptr; // move on to the next tracked device
        }

        vr::RenderModel_TextureMap_t *pTexture;
        while (true)
        {
            error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
            if (error != vr::VRRenderModelError_Loading)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (error != vr::VRRenderModelError_None)
        {
            printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, modelName.c_str());
            vr::VRRenderModels()->FreeRenderModel(pModel);
            return nullptr; // move on to the next tracked device
        }
        //Transfer render model from OpenVR format to our internal Entity forma
        renderEntities[modelName] = vr::createEntity(*pModel, *pTexture);
        renderEntities[modelName]->setProjectionMatPtr(camera->getProjMatPtr());
        renderEntities[modelName]->setViewMatPtr(camera->getViewMatPtr());
        vr::VRRenderModels()->FreeRenderModel(pModel);
        vr::VRRenderModels()->FreeTexture(pTexture);
    }
    return renderEntities.at(modelName);
}
void TrackedDevicesVR::render()
{
	if (HMD->IsInputFocusCapturedByAnotherProcess())
		return;

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (trackedDevices[unDevice])
		{
			if (HMD->IsTrackedDeviceConnected(unDevice))
			{
				if (trackedDevices[unDevice]->getType() == vr::TrackedDeviceClass_Controller)
				{
					if (auto t = std::dynamic_pointer_cast<Controller>(trackedDevices[unDevice]))
					{
						if (Controller::Hand::Left == t->getHand())
						{
							if (renderLeftController)
								trackedDevices[unDevice]->render();
						}
						else if (Controller::Hand::Right == t->getHand())
						{
							if (renderRightController)
								trackedDevices[unDevice]->render();
						}
						else if (Controller::Hand::Invalid == t->getHand())
						{
							if (renderInvalidController)
								trackedDevices[unDevice]->render();
						}
					}
				}
				else if (renderTracker && trackedDevices[unDevice]->getType() == vr::TrackedDeviceClass_GenericTracker)
				{
					trackedDevices[unDevice]->render();
				}
				else if (renderHeadset && trackedDevices[unDevice]->getType() == vr::TrackedDeviceClass_HMD)
				{
					trackedDevices[unDevice]->render();
				}
				else if (renderLighthouses && trackedDevices[unDevice]->getType() == vr::TrackedDeviceClass_TrackingReference)
				{
					trackedDevices[unDevice]->render();
				}
                else if (renderUnknown && trackedDevices[unDevice]->getType() == vr::TrackedDeviceClass_Invalid)
				{
					trackedDevices[unDevice]->render();
				}
			}
		}
	}
}
void TrackedDevicesVR::updatePoses()
{//This method should be left as late as possible before rendering rendermodel
	vr::VRCompositor()->WaitGetPoses(trackedDevicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
	for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount;++i)
	{
		if (trackedDevicePoses[i].bPoseIsValid)
		{
			if (trackedDevices[i])
			{
				glm::mat4 poseMat = vr::toMat4(trackedDevicePoses[i].mDeviceToAbsoluteTracking);                
				//setVelocity
				//setAngularVelocity
				if (i == vr::k_unTrackedDeviceIndex_Hmd)
				{//Invert HMD for camera (not sure why, but this is how it's done)
                    camera->setHMDPose(inverse(poseMat)); 
				}
                
                trackedDevices[i]->setPose(camera->applyWorldMat(poseMat));
			}
		}
	}
}

std::shared_ptr<Controller> TrackedDevicesVR::getController(const unsigned int &no)
{
    std::shared_ptr<Device> s;
    unsigned int j = 0;
    for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
    {
        if (trackedDevices[i] && trackedDevices[i]->getType() == vr::TrackedDeviceClass_Controller)
        {
            if (j++==no)
            {
                s = trackedDevices[i];
            }
        }
    }
    return std::dynamic_pointer_cast<Controller>(s);
}
std::shared_ptr<Controller> TrackedDevicesVR::getLeftController()
{
    unsigned int i = HMD->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
    if (i < vr::k_unMaxTrackedDeviceCount&&i != vr::k_unTrackedDeviceIndex_Hmd)
    {
        if (auto s = std::dynamic_pointer_cast<Controller>(trackedDevices[i]))
        {
            return s;
        }
    }
    return std::shared_ptr<Controller>();
}
std::shared_ptr<Controller> TrackedDevicesVR::getRightController()
{
    unsigned int i = HMD->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
    if (i < vr::k_unMaxTrackedDeviceCount&&i != vr::k_unTrackedDeviceIndex_Hmd)
    {
        if (auto s = std::dynamic_pointer_cast<Controller>(trackedDevices[i]))
        {
            return s;
        }
    }
    return std::shared_ptr<Controller>();
}