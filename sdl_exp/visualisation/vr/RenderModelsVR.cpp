#include "RenderModelsVR.h"
#include "utilVR.h"
#include <chrono>
#include <thread>

RenderModelsVR::RenderModelsVR(vr::IVRSystem *vr_HMD)
    : initState(true)
    , HMD(vr_HMD)
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
    memset(showTrackedDevice, 0, sizeof(showTrackedDevice));
    for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
    {
        addDevice(unTrackedDevice);
    }
}

RenderModelsVR::~RenderModelsVR()
{
    for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
        trackedDeviceToRenderEntity[i].reset();
    renderEntities.clear();
    memset(showTrackedDevice, 0, sizeof(showTrackedDevice));
}

bool RenderModelsVR::addDevice(unsigned int deviceIndex)
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
        trackedDeviceToRenderEntity[deviceIndex] = nullptr;
        showTrackedDevice[deviceIndex] = false;
    }
    else
    {
        trackedDeviceToRenderEntity[deviceIndex] = renderModel;
        showTrackedDevice[deviceIndex] = true;
    }
}
bool RenderModelsVR::removeDevice(unsigned int deviceIndex)
{
    if (deviceIndex >= vr::k_unMaxTrackedDeviceCount)
        return false;
    bool rtn = (bool)trackedDeviceToRenderEntity[deviceIndex];
    trackedDeviceToRenderEntity[deviceIndex] = nullptr;
    showTrackedDevice[deviceIndex] = false;
    return rtn;
}
std::shared_ptr<Entity2> RenderModelsVR::findLoadRenderModel(std::string &modelName)
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
        vr::VRRenderModels()->FreeRenderModel(pModel);
        vr::VRRenderModels()->FreeTexture(pTexture);
    }
    return renderEntities.at(modelName);
}