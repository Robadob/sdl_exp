#ifndef __Skybox_h__
#define __Skybox_h__

#include "Entity.h"
#include "texture/TextureCubeMap.h"

class Skybox : public Entity
{
public:
    Skybox(const char *texturePath = TextureCubeMap::SKYBOX_PATH, float yOffset = 0.0f);
	void render(unsigned int shaderIndex = 0) override;
	void setViewMatPtr(const Camera *camera);
    void setYOffset(float yOffset);
};
#endif //ifndef __Skybox_h__