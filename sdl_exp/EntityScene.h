#ifndef __EntityScene_h__
#define __EntityScene_h__

#include "visualisation/BasicScene.h"
#include "visualisation/Entity.h"
#include "visualisation/Text.h"

#include "visualisation/texture/TextureBuffer.h"
#include "visualisation/model/SceneGraphJoint.h"

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
    std::shared_ptr<Entity> deerModel;
	std::shared_ptr<Entity> colorModel;
	std::shared_ptr<Entity> teapotModel, teapotModel2;
	std::shared_ptr<SceneGraphJoint> teapotJoint, teapotJoint2;
    float tick;
    int polarity;
};

#endif
