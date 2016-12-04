#include "TwoPassScene.h"
#include "visualisation/multipass/FrameBuffer.h"
#include "visualisation/multipass/BackBuffer.h"
#include <glm/gtc/matrix_transform.inl>
//Create content struct
TwoPassScene::SceneContent::SceneContent()
    : deerModel(new Entity(Stock::Models::DEER, 25.0f, { Stock::Shaders::DEPTH, Stock::Shaders::TEXTURE }))
    , sphereModel(new Entity(Stock::Models::SPHERE, 25.0f, { Stock::Shaders::DEPTH, Stock::Shaders::PHONG }))
    , planeModel(new Entity(Stock::Models::PLANE, 100.0f, { Stock::Shaders::DEPTH, Stock::Shaders::PHONG }))
    , spotlightPos(75,100,0)//100 units up, radius of 75
    , spotlightTarget(0)
{
    deerModel->exportModel();
    sphereModel->exportModel();
}
TwoPassScene::TwoPassScene(Visualisation &visualisation)
	: MultiPassScene(visualisation)
	, content(std::make_shared<SceneContent>())
    , sPass(std::make_shared<ShadowPass>(content))
    , cPass(std::make_shared<CompositePass>(content))
	, tick(0.0f)
	, tick2(0.0f)
	, polarity(-1)
{
	//Register models
    registerEntity(content->deerModel);
    registerEntity(content->sphereModel);
    registerEntity(content->planeModel);
	////Register render passes in correct order
	addPass(0, sPass);
	addPass(1, cPass);
    //Put a preview of the depth texture on the HUD
    std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(sPass->getFrameBuffer());
    if (t)
        shadowMapPreview = std::make_shared<Sprite2D>(t->getDepthTextureName(), 256, 256);
    this->visualisation.getHUD()->add(shadowMapPreview, HUD::AnchorV::South, HUD::AnchorH::East);
	//Enable defaults
	this->visualisation.setWindowTitle("MultiPass Render Sample");
	//this->content->deerModel->flipVertexOrder();
}
/*
Called once per frame when Scene animation calls should be
@param Milliseconds since last time this method was called
*/
void TwoPassScene::update(unsigned int frameTime)
{
    //Increment ticks
	this->tick += this->polarity*((frameTime * 60) / 1000.0f)*0.005f;
	this->tick2 += this->polarity*((frameTime * 60) / 1000.0f)*0.05f;
    //Wrap ticks
	this->tick = (float)fmod(this->tick, 360*8);
	this->tick2 = (float)fmod(this->tick2, 360*8);
    //Move spotlight
    const float SPOTLIGHT_RAD = 75.0f;
    this->content->spotlightPos = glm::vec3(SPOTLIGHT_RAD * sin(this->tick), 100, SPOTLIGHT_RAD * cos(this->tick));

	//this->content->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick2*-100));
	//this->content->deerModel->setLocation(glm::vec3(20 * sin(this->tick), 0, 20 * cos(this->tick)));
}

bool TwoPassScene::keypress(SDL_Keycode keycode, int x, int y)
{
	switch (keycode)
	{
	case SDLK_p:
		this->polarity = ++this->polarity>1 ? -1 : this->polarity;
		break;
	default:
		//Permit the keycode to be processed if we haven't handled personally
		return true;
	}
	return false;
}

void TwoPassScene::reload()
{
    //Nothing required
}

TwoPassScene::ShadowPass::ShadowPass(std::shared_ptr<SceneContent> content)
    : RenderPass(std::make_shared<FrameBuffer>(glm::uvec2(1024, 1024), FBAFactory::Disabled(), FBAFactory::ManagedDepthTexture(), FBAFactory::Disabled()))
	, content(content)
{
    //Pass the shadow texture to the second shader of each model
    std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(getFrameBuffer());
    if (t)
    {
        GLuint shadowMap = t->getDepthTextureName();
        content->deerModel->getShaders(1)->addTextureUniform(shadowMap, "_shadowMap", GL_TEXTURE_2D);
        content->sphereModel->getShaders(1)->addTextureUniform(shadowMap, "_shadowMap", GL_TEXTURE_2D);
        content->planeModel->getShaders(1)->addTextureUniform(shadowMap, "_shadowMap", GL_TEXTURE_2D);
    }
    //Camera at spotlightPos looking in spotlightTarget, with up vector looking up y axis
    this->mvMat = glm::lookAt(
        content->spotlightPos,
        content->spotlightTarget,
        glm::vec3(0, 0, 1)
        );
    //Define the entire space the light touches
    this->projMat = glm::ortho(-50, 50, -50, 50, -50, 50);
}
TwoPassScene::CompositePass::CompositePass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<BackBuffer>())
	, content(content)
	
{
}
//Renders the scene to a depth texture from the lights perspective
void TwoPassScene::ShadowPass::render()
{
    content->deerModel->render(0);
    content->sphereModel->render(0);
    content->planeModel->render(0);
}
//Uses the shadow map to render the normal scene
void TwoPassScene::CompositePass::render()
{
    content->deerModel->render(1);
    content->sphereModel->render(1);
    content->planeModel->render(1);
}