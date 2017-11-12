#include "EntityScene.h"
#include <ctime>

/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
    : BasicScene(visualisation)
    , deerModel(Entity::load(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
	, colorModel(Entity::load(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
    , tick(0.0f)
    , polarity(-1)
	, instancedSphere(Entity::load(Stock::Models::ICOSPHERE, 1.0f, Stock::Shaders::INSTANCED))
#ifdef __CUDACC__
    , cuTexBuf(mallocGLInteropTextureBuffer<float>(100, 3))
    , texBuf(TextureBuffer<float>::make(cuTexBuf, true))
#else
    , texBuf(TextureBuffer<float>::make(100, 3))
#endif
{
	if (this->deerModel)
		registerEntity(deerModel);
	if (this->colorModel)
		registerEntity(colorModel);
	if (this->instancedSphere)
		registerEntity(instancedSphere);
    this->setSkybox(true);
    this->visualisation.setWindowTitle("Entity Render Sample");
    this->setRenderAxis(true); 
    srand((unsigned int)time(0));
	if (this->colorModel)
	{
		this->colorModel->rotate(glm::vec3(1.0, 0.0, 0.0), -90);
		this->colorModel->setCullFace(false);
	}
#ifdef __CUDACC__
    cuInit();
#else
    float *tempData = (float*)malloc(sizeof(float) * 3 * 100);
    for (int i = 0; i < 100;i++)
    {
        tempData[(i * 3) + 0] = 100 * (float)sin(i*3.6);
        tempData[(i * 3) + 1] = -50.0f;
        tempData[(i * 3) + 2] = 100 * (float)cos(i*3.6);
    }
    texBuf->setData(tempData);
    free(tempData);
#endif
	if (this->instancedSphere)
	{
		this->instancedSphere->getShaders()->addTexture("_texBuf", texBuf);
		this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
	}
}
/*
Called once per frame when Scene animation calls should be 
@param Milliseconds since last time this method was called
*/
void EntityScene::update(unsigned int frameTime)
{
	float tickLen = this->polarity*((frameTime * 60) / 1000.0f)*0.01f;
	this->tick += tickLen;
    this->tick = (float)fmod(this->tick,360);

	if (this->deerModel)
	{
		this->deerModel->rotate(glm::vec3(0.0, 1.0, 0.0), tickLen*-5);
		this->deerModel->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
	}
#ifdef __CUDACC__
    cuUpdate();
#endif
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
	if (this->colorModel)
		colorModel->render();
	if (this->deerModel)
		deerModel->render();
	if (this->instancedSphere)
		this->instancedSphere->renderInstances(100);
}
/*
Called when the user requests a reload
*/
void EntityScene::reload()
{
	if (this->instancedSphere)
		this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
}

bool EntityScene::keypress(SDL_Keycode keycode, int x, int y)
{
    switch (keycode)
    {
    case SDLK_p:
        this->polarity = ++this->polarity>1 ? -1 : this->polarity;
        break;
	case SDLK_HASH:
		if (this->colorModel)
			this->colorModel->exportModel();
		if (this->deerModel)
			this->deerModel->exportModel();
    default:
        //Permit the keycode to be processed if we haven't handled personally
        return true;
    }
    return false;
}