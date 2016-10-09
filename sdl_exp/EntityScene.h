#ifndef __EntityScene_h__
#define __EntityScene_h__

#include "visualisation/Scene.h"
#include "visualisation/Entity.h"

#include "visualisation/texture/TextureBuffer.h"
#include "visualisation/ComputeShader.h"

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
    void renderParticles();
    void initParticles();
    void *particleData;
    GLuint vbo, fvbo;
    float particleTick;
    glm::mat4 particleTransform;

#ifdef __CUDACC__
    void cuInit();
    void cuUpdate();
    CUDATextureBuffer<float> *cuTexBuf;
#endif
    TextureBuffer<float> texBuf;
    std::shared_ptr<Entity> instancedSphere;
    std::shared_ptr<Entity> deerModel;
    std::shared_ptr<Entity> colorModel;
    std::shared_ptr<Shaders> billboardShaders;
    float tick;
    int polarity;

	ComputeShader *particleSort;
	//This is the next power of 2 after the number of items
	//We need this value 'n' because bitonic networks work over 2^n items
	//It also defines the number of passes required
	int nextPow2 = 0;
	GLint particleCtLoc;
	GLint threadCtLoc;
	GLint hopLoc;
	GLint hop2Loc;
	GLint eyeLoc;
	GLint periodisationLoc;
	GLint directionLoc;
	GLuint ssbo;
};

#endif
