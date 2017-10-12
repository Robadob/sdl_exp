#ifndef __Skybox_h__
#define __Skybox_h__

#include "Entity.h"
#include "texture/TextureCubeMap.h"

class Skybox : public Entity
{
public:
	Skybox(const char *texturePath = TextureCubeMap::SKYBOX_PATH, float yOffset = 0.0f);
	glm::mat4 render(const unsigned int &shaderIndex = 0, glm::mat4 transform = glm::mat4()) override;
	void setViewMatPtr(std::shared_ptr<const Camera> camera);
    void setYOffset(float yOffset);
};
#endif //ifndef __Skybox_h__