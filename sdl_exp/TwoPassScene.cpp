#include "TwoPassScene.h"
#include "visualisation/multipass/FrameBuffer.h"
#include "visualisation/multipass/BackBuffer.h"
#include <glm/gtc/matrix_transform.inl>
#include <glm/gtc/type_ptr.hpp>
//Create content struct
TwoPassScene::SceneContent::SceneContent(std::shared_ptr<LightsBuffer> lights)
	: lights(lights)
	, deerModel(new Entity(Stock::Models::DEER, 25.0f, { Stock::Shaders::LINEAR_DEPTH, Stock::Shaders::PHONG_SHADOW }))
    , sphereModel(new Entity(Stock::Models::SPHERE, 10.0f, { Stock::Shaders::LINEAR_DEPTH, Stock::Shaders::FLAT_SHADOW }))
    , planeModel(new Entity(Stock::Models::PLANE, 100.0f, { Stock::Shaders::LINEAR_DEPTH, Stock::Shaders::PHONG_SHADOW }))
	, bob(new Model("..\\models\\bob\\bob.md5mesh", 35.0f, true, { Stock::Shaders::BONE_LINEAR_DEPTH, Stock::Shaders::BONE_SHADOW }))
    , blur(new GaussianBlur(5,1.75f))
    , pointlightPos(75, 100, 0)//100 units up, radius of 75
    , pointlightTarget(0)
    , shadowDims(2048)
	, shadowIn()
    , shadowOut(Texture2D::make(shadowDims, { GL_RED, GL_R32F, sizeof(float), GL_FLOAT }, nullptr, Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR | Texture::FILTER_MAG_LINEAR | Texture::WRAP_CLAMP_TO_EDGE))
{
	//planeModel->setMaterial(Stock::Materials::RED_PLASTIC);
	//sphereModel->setMaterial(Stock::Materials::COPPER);
    deerModel->exportModel();
    sphereModel->exportModel();
    sphereModel->setLocation(glm::vec3(10, 5, 10));
	bob->setLocation(glm::vec3(-20,0,10));
}
TwoPassScene::TwoPassScene(Visualisation &visualisation)
	: MultiPassScene(visualisation)
	, content(std::make_shared<SceneContent>(Lights()))
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
	registerEntity(content->bob);
	////Register render passes in correct order
	addPass(0, sPass);
	addPass(1, cPass);
    //Put a preview of the depth texture on the HUD
	shadowMapPreview = std::make_shared<Sprite2D>(content->shadowOut, std::make_shared<Shaders>(Stock::Shaders::SPRITE2D_HEAT), glm::uvec2(256, 256));
    this->visualisation.getHUD()->add(shadowMapPreview, HUD::AnchorV::South, HUD::AnchorH::East);
	//Enable defaults
	this->visualisation.setWindowTitle("MultiPass Render Sample");

	SpotLight p = Lights()->addSpotLight();
	p.Position(this->content->pointlightPos);
	p.ConstantAttenuation(1.0f);
	p.Direction(this->content->pointlightTarget - this->content->pointlightPos);
	p.CutOff(15.0f);
	p.Exponent(70.0f);

    //Spotlight camera at pointlightPos looking in pointlightTarget, with up vector looking up y axis
    //These must be set *AFTER* the parent entities have been registered (need to fiddle with shaders to better handle this use case)
	auto deer0 = content->deerModel->getShaders(0);
	auto sphere0 = content->sphereModel->getShaders(0);
	auto plane0 = content->planeModel->getShaders(0);
	auto bob0 = content->bob->getShaders(0);
	auto deer1 = content->deerModel->getShaders(1);
	auto sphere1 = content->sphereModel->getShaders(1);
	auto plane1 = content->planeModel->getShaders(1);
	auto bob1 = content->bob->getShaders(1);

	deer0->setViewMatPtr(&this->content->pointlightV);
	sphere0->setViewMatPtr(&this->content->pointlightV);
	plane0->setViewMatPtr(&this->content->pointlightV);
	bob0->setViewMatPtr(&this->content->pointlightV);
	//
	deer1->addDynamicUniform("spotlightViewMat", &this->content->pointlightV);
	sphere1->addDynamicUniform("spotlightViewMat", &this->content->pointlightV);
	plane1->addDynamicUniform("spotlightViewMat", &this->content->pointlightV);
	bob1->addDynamicUniform("spotlightViewMat", &this->content->pointlightV);
    //Define the entire space the light touches
    this->content->pointlightP = glm::ortho(-71.0, 71.0, -71.0, 71.0, 82.0, 180.0);
	//Specify where we are rendering from to gen linear depth map
	deer0->setProjectionMatPtr(&this->content->pointlightP);
	sphere0->setProjectionMatPtr(&this->content->pointlightP);
	plane0->setProjectionMatPtr(&this->content->pointlightP);
	bob0->setProjectionMatPtr(&this->content->pointlightP);
	//
	deer1->addDynamicUniform("spotlightProjectionMat", &this->content->pointlightP);
	sphere1->addDynamicUniform("spotlightProjectionMat", &this->content->pointlightP);
	plane1->addDynamicUniform("spotlightProjectionMat", &this->content->pointlightP);
	bob1->addDynamicUniform("spotlightProjectionMat", &this->content->pointlightP);
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
    const float SPOTLIGHT_RAD = 70.0f;
    this->content->pointlightPos = glm::vec3(SPOTLIGHT_RAD * sin(this->tick), 100, SPOTLIGHT_RAD * cos(this->tick));
    this->content->pointlightV = glm::lookAt(
        content->pointlightPos,
        content->pointlightTarget,
        glm::vec3(0, 1, 0)
        );

	if (!this->bobPause)
		this->content->bob->update((SDL_GetTicks() / 1000.0f) - this->bobAnimOffset);
	//Attach light source to moving light
	Lights()->getSpotLight(0).Position(this->content->pointlightPos);
	Lights()->getSpotLight(0).Direction(this->content->pointlightTarget - this->content->pointlightPos);
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
	case SDLK_o:
		this->bobPause = !this->bobPause;
		this->bobAnimOffset = (SDL_GetTicks() / 1000.0f) - this->bobAnimOffset;
		break;
	default:
		//Permit the keycode to be processed if we haven't handled personally
		return true;
	}
	return false;
}

void TwoPassScene::reload()
{
    content->blur->reload();
}

TwoPassScene::ShadowPass::ShadowPass(std::shared_ptr<SceneContent> content)
    : RenderPass(std::make_shared<FrameBuffer>(content->shadowDims, FBAFactory::ManagedColorTexture(GL_R32F, GL_RED, GL_FLOAT), FBAFactory::ManagedDepthRenderBuffer(), FBAFactory::Disabled(), 0, true, glm::vec3(1.0f)))
	, content(content)
{
    //Pass the shadow texture to the second shader of each model
    std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(getFrameBuffer());
    if (t)
    {
		content->shadowIn = std::dynamic_pointer_cast<Texture2D>(t->getColorTexture());
        //content->shadowOut = content->shadowIn;//Uses the pre blur shadow map (aka hard shadows)
    }
	content->deerModel->getShaders(1)->addTexture("_shadowMap", content->shadowOut);
	content->sphereModel->getShaders(1)->addTexture("_shadowMap", content->shadowOut);
	content->planeModel->getShaders(1)->addTexture("_shadowMap", content->shadowOut);
	content->bob->getShaders(1)->addTexture("_shadowMap", content->shadowOut);
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
	content->bob->render(0);
}
//Uses the shadow map to render the normal scene
void TwoPassScene::CompositePass::render()
{
	//Slightly blur shadow map to reduce harshness of edges
	content->blur->blurR32F(content->shadowIn, content->shadowOut);
	//Generate mip-map
	content->shadowOut->updateMipMap();
	//Render models using shadow map
    content->deerModel->render(1);
    content->sphereModel->render(1);
	content->planeModel->render(1);
	content->bob->render(1);
	//Render something at the lights location
	content->lights->render();
}