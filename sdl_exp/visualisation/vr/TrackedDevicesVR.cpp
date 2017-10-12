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

void TrackedDevicesVR::update()
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
			{//Button has been pressed
				//event.data.controller.button identifies button
				if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu))
				{
					printf("Menu button pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_Grip))
				{
					printf("Grip button pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))
				{
					printf("Touchpad pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
				{
					printf("Trigger pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_Axis2))
				{
					printf("Axis 2(spare button) pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_Axis3))
				{
					printf("Axis 3(spare button) pressed.\n");
				}
				else if (event.data.controller.button == vr::ButtonMaskFromId(vr::k_EButton_Axis4))
				{
					printf("Axis 4(spare button) pressed.\n");
				}
			}
			case vr::VREvent_ButtonTouch:
			{//Button has been touched (always occurs before press)
				//event.data.controller.button identifies button
			}
			case vr::VREvent_MouseMove:
			{//Simulated mouse has moved (does this mean controller?)
				//event.data.mouse.button == vr::VRMouseButton_Left, vr::VRMouseButton_Right, vr::VRMouseButton_Middle
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
	case vr::TrackedDeviceClass_GenericTracker:
	case vr::TrackedDeviceClass_HMD:
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
	updatePoses();
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
						else if (Controller::Hand::Left == t->getHand())
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
				else if (renderUnknown)
				{
					trackedDevices[unDevice]->render();
				}
			}
		}
	}
}
TrackedDevicesVR::Device::Device(vr::IVRSystem *vr_HMD, unsigned int id, std::shared_ptr<Entity2> model)
	: renderSceneGraph(true)
	, unPacketNum(0)
	, vr_HMD(vr_HMD)
	, id(id)
	, model(model)//Presume id 0 == HMD
	, type(vr_HMD->GetTrackedDeviceClass(id))
{
}

void TrackedDevicesVR::Device::update()
{
	vr::VRControllerState_t _state;
	if (vr_HMD->GetControllerState(id, &_state, sizeof(_state)))
	{//https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetControllerState
		if (unPacketNum != _state.unPacketNum)
		{//State has changed
			unPacketNum = _state.unPacketNum;
			//do something with controller state
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
				trackedDevices[i]->setPose(poseMat);
				//setVelocity
				//setAngularVelocity
				if (i == vr::k_unTrackedDeviceIndex_Hmd)
				{
					camera->setHMDPose(inverse(poseMat));
				}
			}
		}
	}
}
void TrackedDevicesVR::Device::render()
{
	if (renderSceneGraph)
		model->renderSceneGraph(0, poseMat);
	else
		model->render(0, poseMat);
}