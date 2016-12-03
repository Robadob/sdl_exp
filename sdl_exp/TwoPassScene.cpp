#include "TwoPassScene.h"
#include "visualisation/multipass/FrameBuffer.h"
#include "visualisation/multipass/BackBuffer.h"
#include <glm/gtc/matrix_transform.inl>
//Create content struct
TwoPassScene::SceneContent::SceneContent()
	: deerModel(new Entity(Stock::Models::DEER, 10.0f, { Stock::Shaders::TEXTURE, Stock::Shaders::VELOCITY }))
	, skybox(new Skybox())
	, axis(new Axis(1))
	, compositeShader(std::make_shared<Shaders>(Stock::Shaders::VELOCITY_COMPOSITOR))
{ }
TwoPassScene::TwoPassScene(Visualisation &visualisation)
	: MultiPassScene(visualisation)
	, content(std::make_shared<SceneContent>())
	, vPass(std::make_shared<VelocityPass>(content))
	, cPass(std::make_shared<ColorPass>(content))
	, mbcPass(std::make_shared<MotionBlurCompositePass>(content))
	, tick(0.0f)
	, tick2(0.0f)
	, polarity(-1)
{
	//Register models
	registerEntity(content->deerModel);
	registerEntity(content->skybox);
	//Register render passes in correct order
	addPass(0, vPass);
	addPass(1, cPass);
	addPass(2, mbcPass);
	//Share render textures from vPass/cPass to mbcPass
	std::shared_ptr<FrameBuffer> t = std::dynamic_pointer_cast<FrameBuffer>(vPass->getFrameBuffer());
	if (t)
		mbcPass->setVelocityTex(t->getColorTextureName());
	t = std::dynamic_pointer_cast<FrameBuffer>(cPass->getFrameBuffer());
	if (t)
	mbcPass->setColorTex(t->getColorTextureName());
	t = std::dynamic_pointer_cast<FrameBuffer>(cPass->getFrameBuffer());
	if (t)
	mbcPass->setDepthTex(t->getDepthStencilTextureName());
	//Enable defaults
	this->visualisation.setWindowTitle("MultiPass Render Sample");
	this->content->deerModel->flipVertexOrder();
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
	case SDLK_F9:
		cPass->toggleSkybox();
		break;
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

TwoPassScene::VelocityPass::VelocityPass(std::shared_ptr<SceneContent> content)
	//: RenderPass(std::make_shared<BackBuffer>(true, glm::vec3(0.5)))
    : RenderPass(std::make_shared<FrameBuffer>(FBAFactory::ManagedColorTextureRGB(), FBAFactory::ManagedDepthStencilRenderBuffer24()))
	, content(content)
{
}
TwoPassScene::ColorPass::ColorPass(std::shared_ptr<SceneContent> content)
    : RenderPass(std::make_shared<FrameBuffer>(FBAFactory::ManagedColorTextureRGB(), FBAFactory::ManagedDepthStencilTexture32()))
	, renderSkybox(true)
	, renderAxis(false)
	, content(content)
{ }
TwoPassScene::MotionBlurCompositePass::MotionBlurCompositePass(std::shared_ptr<SceneContent> content)
	: RenderPass(std::make_shared<BackBuffer>())
	, vTex(0)
    , cTex(0)
    , dTex(0)
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
void TwoPassScene::MotionBlurCompositePass::setVelocityTex(GLuint tex)
{
	vTex = tex;
	//Bind to shader
	content->compositeShader->addTextureUniform(tex, "_velocityTex", GL_TEXTURE_2D_MULTISAMPLE);
};
void TwoPassScene::MotionBlurCompositePass::setColorTex(GLuint tex)
{
	cTex = tex;
	//Bind to shader
    content->compositeShader->addTextureUniform(tex, "_colorTex", GL_TEXTURE_2D_MULTISAMPLE);
};
void TwoPassScene::MotionBlurCompositePass::setDepthTex(GLuint tex)
{
	dTex = tex;
	//Bind to shader
    content->compositeShader->addTextureUniform(tex, "_depthTex", GL_TEXTURE_2D_MULTISAMPLE);
};
//Renders the scene in order with the velocity shader to a texture
void TwoPassScene::VelocityPass::render()
{
	//if (renderSkybox)
	//	content->skybox->render(1);
	//if (renderAxis)
	//content->axis->render(1);
	content->deerModel->render(1);
}
//Renders the scene normally to a texture
void TwoPassScene::ColorPass::render()
{
	if (renderSkybox)
		content->skybox->render(0);
	if (renderAxis)
		content->axis->render();
	content->deerModel->render(0);
}
//Uses the velocity texture to blend the previously rendered frame
void TwoPassScene::MotionBlurCompositePass::render()
{
	//Render a fullscreen rectangle
	frameEnt->render();
	////Render any non motionblur elements
}