#ifndef __EntityScene_h__
#define __EntityScene_h__

#include "visualisation/BasicScene.h"
#include "visualisation/Entity.h"
#include "visualisation/Text.h"
#include "visualisation/model/Model.h"

#include "visualisation/texture/TextureBuffer.h"
/*
Example scene to demonstrate how to use SDL_EXP
*/
class EntityScene : public BasicScene
{
public:
    EntityScene(Visualisation &visualisation);

    void render() override;
    void reload() override;
    void update(unsigned int frameTime) override;
    bool keypress(SDL_Keycode keycode, int x, int y) override;
private:
#ifdef __CUDACC__
    void cuInit();
    void cuUpdate();
    CUDATextureBuffer<float> *cuTexBuf;
#endif
    std::shared_ptr<TextureBuffer<float>> texBuf;
    std::shared_ptr<Entity> instancedSphere;
	const unsigned int INSTANCE_COUNT;
    std::shared_ptr<Entity> deerModel;
	std::shared_ptr<Entity> colorModel;
	std::shared_ptr<Entity> mirrorModel;
	std::shared_ptr<Model> bob;
    float tick;
	int polarity;
	bool bobPause = false;
	float bobAnimOffset = 0.0f;
};

#endif
