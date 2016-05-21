#ifndef __EntityScene_h__
#define __EntityScene_h__

#include "visualisation/Scene.h"
#include "visualisation/Entity.h"

#include "visualisation/texture/TextureBuffer.h"
/*
Example scene to demonstrate how to use SDL_EXP
*/
class EntityScene : Scene
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
    TextureBuffer<float> texBuf;
    std::shared_ptr<Entity> instancedSphere;
    std::shared_ptr<Entity> icosphere;
    std::shared_ptr<Entity> colorModel;
    float tick;
    int polarity;
};

#endif
