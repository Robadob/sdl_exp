#include "BasicScene.h"
#include "shader/lights/LightsBuffer.h"

BasicScene::BasicScene(ViewportExt& vis)
	: Scene(vis)
	, renderAxisState(true)
	, renderSkyboxState(true)
	, axis(std::make_shared<Axis>(25.0f))
	, skybox(std::make_unique<Skybox>())
	, lighting(std::make_shared<LightsBuffer>(vis.getCamera()->getViewMatPtr()))
{
	registerEntity(axis);
	this->skybox->setViewMatPtr(this->visualisation.getCamera());
    this->skybox->setProjectionMatPtr(this->visualisation.getProjectionMatPtr());
	this->skybox->setYOffset(-1.0f);
}
void BasicScene::registerEntity(std::shared_ptr<Renderable> ent)
{
	if (ent)
	{
		//Store value for later
		entities.push_back(ent);
		//Setup matrices
		ent->setViewMatPtr(this->visualisation.getCamera());
		ent->setProjectionMatPtr(&this->visualisation);
		ent->setLightsBuffer(this->lighting);
	}
	else
		fprintf(stderr, "Can't register a null entity!\n");
}
void BasicScene::_render()
{
	//Update lighting buffer
	lighting->update();
    BackBuffer::useStatic();
	if (this->renderSkyboxState)
		this->skybox->render();
	if (this->renderAxisState)
		this->axis->render();
	render();
}
bool BasicScene::_keypress(SDL_Keycode keycode, int x, int y) 
{
	//Pass key events to the scene and skip handling if false is returned 
	if (!keypress(keycode, x, y))
		return false;
	switch (keycode){
	case SDLK_F9:
		this->setSkybox(!this->renderSkyboxState);
		break;
	default:
		return true;
	}
	return false;
}
void BasicScene::_reload() 
{
	for (std::vector<std::shared_ptr<Renderable>>::iterator i = entities.begin(); i != entities.end(); i++)
	{
		(*i)->reload();
	}
	reload();
}
void BasicScene::setSkybox(bool state){
	this->renderSkyboxState = state;
}
void BasicScene::setRenderAxis(bool state){
    this->renderAxisState = state;
}
//void BasicScene::defaultLighting(){
    //glEnable(GL_LIGHT0);
    //glm::vec3 eye = this->visualisation.getCamera()->getEye();
    //float lightPosition[4] = { eye.x, eye.y, eye.z, 1 };
    //float amb[4] = { 0.8f, 0.8f, 0.8f, 1 };
    //float diffuse[4] = { 0.2f, 0.2f, 0.2f, 1 };
    //float white[4] = { 1, 1, 1, 1 };
    //glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    //glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    // Spotlight stuff
    //float angle = 180.0f;
    //glm::vec3 look = this->camera.getLook();
    // float direction[4] = { look.x, look.y, look.z, 0 };
    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
//}