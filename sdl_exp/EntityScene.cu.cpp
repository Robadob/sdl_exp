#include "EntityScene.h"
#include <ctime>
#include "visualisation/TowerCrane.h"

/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
    : BasicScene(visualisation)
    , deerModel(Entity::load(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
	, colorModel(Entity::load(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
	, teapotModel(Entity::load(Stock::Models::TEAPOT, 1.0f, Stock::Shaders::PHONG))
	, teapotModel2(Entity::load(Stock::Models::TEAPOT, 1.0f, Stock::Shaders::PHONG))
    , tick(0.0f)
    , polarity(-1)
	, instancedSphere(Entity::load(Stock::Models::ICOSPHERE, 1.0f, Stock::Shaders::INSTANCED))
#ifdef __CUDACC__
    , cuTexBuf(mallocGLInteropTextureBuffer<float>(100, 3))
    , texBuf(TextureBuffer<float>::make(cuTexBuf, true))
#else
    , texBuf(TextureBuffer<float>::make(100, 3))
#endif
	, teapotJoint(SGJoint::make())
	, teapotJoint2(SGJoint::make())
	, crane(TowerCrane::make())
{
	if (this->deerModel)
		registerEntity(deerModel);
	if (this->colorModel)
		registerEntity(colorModel);
	if (this->instancedSphere)
		registerEntity(instancedSphere);
	if (this->teapotModel)
	{
		registerEntity(teapotModel);
		teapotModel->setColor(glm::vec3(0, 1, 0));
	}
	if (this->crane)
	{
		registerEntity(crane);
	}
	if (this->teapotModel2)
		registerEntity(teapotModel2);
    this->setSkybox(true);
    this->visualisation.setWindowTitle("Entity Render Sample");
    this->setRenderAxis(true); 
    srand((unsigned int)time(0));
	if (this->colorModel)
	{
		this->colorModel->rotate(glm::vec3(1.0, 0.0, 0.0), -90);
		this->colorModel->setCullFace(false);
	}
	if(this->deerModel)
	{

		//Attach teapoint Joint to 0,7,0
		this->deerModel->attach(this->teapotJoint, "teapot_joint", glm::vec3(0, 7, 0));
		//Attach teapot (0,1,0) to (0,7,0) = (0,6,0)
		this->teapotJoint->attach(this->teapotModel, "teapot1", glm::vec3(0,1,0));
		//Attach teapot (0,-1,0) to (0,7,0) = (0,8,0)
		this->teapotModel->attach(this->teapotJoint2, "teapot_joint2", glm::vec3(0, 1, 0));
		this->teapotJoint2->attach(this->teapotModel2, "teapot2", glm::vec3(0, 0, 0));
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
	frameTime = 12;
	const float CIRCLE_RAD = 10.0f;
	const float MS_PER_CIRCLE = (float)(2 * M_PI) / 10000;
	const float MS_PER_SPIN_DIV_2PI = (float)(2 * M_PI) / 5000;
	this->tick += this->polarity*MS_PER_CIRCLE*frameTime;
	this->tick = (float)fmod((2 * M_PI)+this->tick, 2 * M_PI);//FMOD can't handle negatives

	if (this->deerModel)
	{
		this->deerModel->rotate(glm::vec3(0.0, 1.0, 0.0), -(float)this->polarity*frameTime*MS_PER_SPIN_DIV_2PI);
		this->deerModel->setLocation(glm::vec3(CIRCLE_RAD * sin(this->tick), 0, CIRCLE_RAD * cos(this->tick)));
	}
#ifdef __CUDACC__
    cuUpdate();
#endif
	this->teapotJoint->rotate(glm::vec3(1.0, 0.0, 0.0), frameTime*MS_PER_SPIN_DIV_2PI);
	this->teapotJoint2->rotate(glm::vec3(0.0, 0.0, 1.0), frameTime*MS_PER_SPIN_DIV_2PI);
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
	if (this->colorModel)
		colorModel->render();
	if (this->deerModel)
		deerModel->renderSceneGraph();
	if (this->instancedSphere)
		this->instancedSphere->renderInstances(100);
	if (this->crane)
		this->crane->renderSceneGraph();
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