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
        glm::vec3 _min = glm::vec3(FLT_MAX);
        glm::vec3 _max = glm::vec3(-FLT_MAX);
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
        {
            memcpy(vData + i, &vrModel.rVertexData[i].vPosition.v, sizeof(glm::vec3));
            _min = glm::min(_min, vData[i]);
            _max = glm::max(_max, vData[i]);
        }
        printf("Model Dims: (%.3f, %.3f, %.3f)\n", _max.x - _min.x, _max.y - _min.y, _max.z - _min.z);
        //Copy normals
        glm::vec3 *nData = (glm::vec3*)(vData + vrModel.unVertexCount);
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
            memcpy(nData + i, &vrModel.rVertexData[i].vNormal.v, sizeof(glm::vec3));
        //Copy tex coords
        glm::vec2 *tcData = (glm::vec2*)(nData + vrModel.unVertexCount);
        for (unsigned int i = 0; i < vrModel.unVertexCount; ++i)
            memcpy(tcData + i, &vrModel.rVertexData[i].rfTextureCoord, sizeof(glm::vec2));

        auto rtn = std::make_shared<Entity2>(
            (float*)vData, 3,
            (float*)nData, 3,
            nullptr, 0,
            (float*)tcData, 2,
            vrModel.unVertexCount,
            (void*)vrModel.rIndexData, sizeof(uint16_t), 3, vrModel.unTriangleCount,
            std::make_shared<Texture2D>(
                    (const char *)nullptr, 
                    (void*)vrDiffuseTexture.rubTextureMapData,
                    vrDiffuseTexture.unWidth * vrDiffuseTexture.unHeight * 4 * sizeof(unsigned char),
                    vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE)
                );
        free(newData);
        return rtn;
	}
	inline glm::mat4 toMat4(const vr::HmdMatrix34_t &matPose)
    {
		return glm::mat4(
			matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
			matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
			matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
			matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
			);
    }
}

#endif //__utilVR_h__