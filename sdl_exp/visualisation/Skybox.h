#ifndef __Skybox_h__
#define __Skybox_h__

#include "Entity.h"
#include "texture/TextureCubeMap.h"

class Skybox : public Entity
{
public:
    Skybox(const char *texturePath = TextureCubeMap::SKYBOX_PATH, float yOffset = 0.0f);
    void renderInstances(int count, unsigned int shaderIndex = 0) override;
    void depthRenderInstances(const GLenum &glDepthFunc, int count, unsigned int shaderIndex = 0);
    void render(const unsigned int &shaderIndex = UINT_MAX, const glm::mat4 &transform = glm::mat4(1)) override;
    void depthRender(const GLenum &glDepthFunc, const unsigned int &shaderIndex = UINT_MAX, const glm::mat4 &transform = glm::mat4(1));
	void setViewMatPtr(std::shared_ptr<const Camera> camera) override;
    void setYOffset(float yOffset);
};
#endif //ifndef __Skybox_h__