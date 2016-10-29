#include "TwoPassScene.h"
TwoPassScene::SceneContent::SceneContent()
	: deerModel(new Entity(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
	, skybox(new Skybox())
	, axis(new Axis(1))
{ }
TwoPassScene::TwoPassScene(Visualisation &visualisation)
	: MultiPassScene(visualisation)
	, content(std::make_shared<SceneContent>())
	, tick(0.0f)
	, polarity(-1)
{
	registerEntity(content->deerModel);
	sPass->setSkybox(true);
	sPass->setAxis(true);
	this->visualisation.setWindowTitle("MultiPass Render Sample");
	this->content->deerModel->flipVertexOrder();
}
/*
Called once per frame when Scene animation calls should be
@param Milliseconds since last time this method was called
*/
void TwoPassScene::update(unsigned int frameTime)
{
	this->tick += this->polarity*((frameTime * 60) / 1000.0f)*0.01f;
	this->tick = (float)fmod(this->tick, 360);
	this->content->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick*-100));
	this->content->deerModel->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
}

bool TwoPassScene::keypress(SDL_Keycode keycode, int x, int y)
{
	return true;
}

void TwoPassScene::reload()
{
	
}