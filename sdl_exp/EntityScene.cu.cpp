#include "EntityScene.h"
#include <ctime>
/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
	: BasicScene(visualisation)
	, deerModel(new Entity(Stock::Models::DEER, 10.0f, Stock::Shaders::PHONG))
	, colorModel(new Entity(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
	, tick(0.0f)
	, polarity(-1)
	, instancedSphere(new Entity(Stock::Models::ICOSPHERE, 1.0f, Stock::Shaders::INSTANCED_FLAT))
	, INSTANCE_COUNT(100)
#ifdef __CUDACC__
	, cuTexBuf(mallocGLInteropTextureBuffer<float>(100, 3))
	, texBuf(TextureBuffer<float>::make(cuTexBuf, true))
#else
	, texBuf(TextureBuffer<float>::make(100, 3))
#endif
	, bob(std::make_shared<Model>("..\\models\\bob\\bob.md5mesh", 10.0f))
{
	registerEntity(deerModel);
	registerEntity(colorModel);
	registerEntity(instancedSphere);
	registerEntity(bob);
	this->setSkybox(true);
	this->visualisation.setWindowTitle("Entity Render Sample");
	this->setRenderAxis(true);
	srand((unsigned int)time(0));
	this->colorModel->setRotation(glm::vec4(1.0, 0.0, 0.0, -90));
	this->colorModel->setCullFace(false);
#ifdef __CUDACC__
	cuInit();
#else
	const float PI = 3.14159265f * 2.0f / INSTANCE_COUNT;
	float *tempData = (float*)malloc(sizeof(float) * 3 * INSTANCE_COUNT);
	for (unsigned int i = 0; i < INSTANCE_COUNT; i++)
	{
		tempData[(i * 3) + 0] = INSTANCE_COUNT * (float)sin(i*PI);
		tempData[(i * 3) + 1] = INSTANCE_COUNT/-2.0f;
		tempData[(i * 3) + 2] = INSTANCE_COUNT * (float)cos(i*PI);
	}
	texBuf->setData(tempData);
	free(tempData);
#endif
	this->instancedSphere->getShaders()->addTexture("_texBuf", texBuf);
	glm::vec3 color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	this->instancedSphere->setMaterial(color / 5.0f, color, glm::vec3(1.0f));
	//Approx sun point light 
	//Really wants to be abitrary directional light, but putting it really far out with constant attenuation should also work
	DirectionalLight p = Lights()->addDirectionalLight();
	p.Direction(glm::normalize(glm::vec3(-1, 0, 1)));
	p.Ambient(glm::vec3(0.89f, 0.64f, 0.36f));
	p.Diffuse(glm::vec3(1.0f, 0.75f, 0.39f));//else 1.0f, 1.0f, 0.49f (more yellow, less orange)
	p.Specular(glm::vec3(1, 1, 1));
	p.ConstantAttenuation(1.0f);
	PointLight _p = Lights()->addPointLight();
	_p.Ambient(glm::vec3(0.0f));
	_p.Diffuse(glm::vec3(0.5f));
	_p.Specular(glm::vec3(0.02f));
	_p.ConstantAttenuation(0.5f);
}
/*
Called once per frame when Scene animation calls should be 
@param Milliseconds since last time this method was called
*/
void EntityScene::update(const unsigned int &frameTime)
{
    this->tick += this->polarity*((frameTime*60)/1000.0f)*0.01f;
    this->tick = (float)fmod(this->tick,360);
    this->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick*-100));
    this->deerModel->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
#ifdef __CUDACC__
    cuUpdate();
#endif
	if (!this->bobPause)
		this->bob->update((SDL_GetTicks() / 1000.0f) - this->bobAnimOffset);

	//Emulate full bright, attach the one light source to the camera
	auto p = Lights()->getPointLight(1);
	p.Position(visualisation.getCamera()->getEye());
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
    colorModel->render();
    deerModel->render();
	this->instancedSphere->renderInstances(INSTANCE_COUNT);

	bob->render();
	bob->renderSkeleton();
}
/*
Called when the user requests a reload
*/
void EntityScene::reload()
{
	glm::vec3 color = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	this->instancedSphere->setMaterial(color / 5.0f, color, glm::vec3(0), 0.0f);
}

bool EntityScene::keypress(SDL_Keycode keycode, int x, int y)
{
    switch (keycode)
    {
    case SDLK_p:
        this->polarity = ++this->polarity>1 ? -1 : this->polarity;
		break;
	case SDLK_o:
		this->bobPause = !this->bobPause;
		this->bobAnimOffset = (SDL_GetTicks() / 1000.0f) - this->bobAnimOffset;
		break;
    case SDLK_HASH:
        this->colorModel->exportModel();
        this->deerModel->exportModel();
    default:
        //Permit the keycode to be processed if we haven't handled personally
        return true;
    }
    return false;
}