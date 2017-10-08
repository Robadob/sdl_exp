#ifndef __utilVR_h__
#define __utilVR_h__
#include <string>
#include <openvr/openvr.h>
#include "../Entity2.h"

namespace vr
{
    inline std::string getDeviceString(vr::IVRSystem *hmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = nullptr)
    {
        uint32_t unRequiredBufferLen = hmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
        if (unRequiredBufferLen == 0)
            return "";

        char *pchBuffer = new char[unRequiredBufferLen];
        unRequiredBufferLen = hmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
        std::string sResult = pchBuffer;
        delete[] pchBuffer;
        return sResult;
    }

    inline std::shared_ptr<Entity2> createEntity(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture)
    {

        //Remove buffer interleaving
        float *newData = (float *)malloc(sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount);
        //Copy vertices
        glm::vec3 *vData = (glm::vec3*)newData;
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
            memcpy(vData + i, &vrModel.rVertexData[i].vPosition.v, sizeof(glm::vec3));
        //Copy normals
        glm::vec3 *nData = (glm::vec3*)vData + vrModel.unVertexCount;
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
            memcpy(vData + i, &vrModel.rVertexData[i].vNormal.v, sizeof(glm::vec3));
        //Copy tex coords
        glm::vec2 *tcData = (glm::vec2*)nData + vrModel.unVertexCount;
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
            memcpy(vData + i, &vrModel.rVertexData[i].rfTextureCoord, sizeof(glm::vec2));

        auto rtn = std::make_shared<Entity2>(
            (float*)vData, 3,
            (float*)nData, 3,
            nullptr, 0,
            (float*)tcData, 2,
            vrModel.unVertexCount,
            (void*)vrModel.rIndexData, sizeof(uint16_t), 3, vrModel.unTriangleCount * 3,
            std::make_shared<Texture2D>(
                    (const char *)nullptr, 
                    (void*)vrDiffuseTexture.rubTextureMapData,
                    vrDiffuseTexture.unWidth * vrDiffuseTexture.unHeight * 4 * sizeof(unsigned char),
                    vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE)
                );
        free(newData);
        return rtn;
    }
}

#endif //__utilVR_h__