#include "TwoPassScene.h"
#include "visualisation/multipass/FrameBuffer.h"
#include "visualisation/multipass/BackBuffer.h"
#include <glm/gtc/matrix_transform.inl>
//Create content struct
TwoPassScene::SceneContent::SceneContent()
    : deerModel(new Entity(Stock::Models::DEER, 25.0f, { Stock::Shaders::SHADOW_SPOT, Stock::Shaders::TEXTURE }))
    , sphereModel(new Entity(Stock::Models::SPHERE, 25.0f, { Stock::Shaders::SHADOW_SPOT, Stock::Shaders::PHONG }))
    , planeModel(new Entity(Stock::Models::PLANE, 100.0f, { Stock::Shaders::SHADOW_SPOT, Stock::Shaders::PHONG }))
{ }
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
	//Register render passes in correct order
	addPass(0, sPass);
	addPass(1, cPass);
	//Share render textures from sPass to cPass
    std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(sPass->getFrameBuffer());
    shadowMapPreview = std::make_shared<Sprite2D>(t->getDepthTextureName(), 256, 256);
	if (t)
		cPass->setShadowMap(t->getDepthTextureName());

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
	this->tick += this->polarity*((frameTime * 60) / 1000.0f)*0.005f;
	this->tick2 += this->polarity*((frameTime * 60) / 1000.0f)*0.05f;
	this->tick = (float)fmod(this->tick, 360*8);
	this->tick2 = (float)fmod(this->tick2, 360*8);
	this->content->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick2*-100));
	this->content->deerModel->setLocation(glm::vec3(20 * sin(this->tick), 0, 20 * cos(this->tick)));
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
	content->compositeShader->reload();
}

TwoPassScene::ShadowPass::ShadowPass(std::shared_ptr<SceneContent> content)
    : RenderPass(std::make_shared<FrameBuffer>(glm::uvec2(1024,1024), FBAFactory::Disabled(), FBAFactory::ManagedDepthTexture(), FBAFactory::Disabled(),1.0f,0))
	, content(content)
{
}
TwoPassScene::CompositePass::CompositePass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<BackBuffer>())
    , shadowMap(0)
	, content(content)
	
{
	//Camera at origin looking down y axis, with up vector looking up z axis
	this->mvMat = glm::lookAt(
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1)
		);
	//2 width, (-)2 height
	this->projMat = glm::ortho(0, 1, 0, 1, 0, 1);

	content->compositeShader->setModelViewMatPtr(&mvMat);
	content->compositeShader->setProjectionMatPtr(&projMat);
	frameEnt = std::make_shared<Entity>(Stock::Models::FRAME, 1.0f, content->compositeShader);
	//sampleTex = std::make_shared<Texture2D>("../textures/deer.tga");
	//Make vertex and face vbos of this
}
void TwoPassScene::ShadowPass::setShadowMap(GLuint tex)
{
    shadowMap = tex;
	//Bind to shader
	content->compositeShader->addTextureUniform(tex, "_shadowMap", GL_TEXTURE_2D);
};
//Renders the scene to a depth texture from the lights perspective
void TwoPassScene::CompositePass::render()
{
	content->deerModel->render(0);
}
//Uses the shadow map to render the normal scene
void TwoPassScene::CompositePass::render()
{
    content->deerModel->render(1);
}