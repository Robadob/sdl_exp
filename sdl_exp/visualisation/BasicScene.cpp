#include "BasicScene.h"
#include "shader/lights/LightsBuffer.h"
#include "model/Model.h"

BasicScene::BasicScene(Visualisation& vis)
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
void BasicScene::enableEnvironmentMap(std::shared_ptr<RenderableAdv> entAdv, const unsigned int &dynamicEnvMapWidthHeight)
{
	//Don't allow duplicate envMaps
	disableEnvironmentMap(entAdv);
	//Setup dynamicEnvMap
	if (dynamicEnvMapWidthHeight)
	{
		//Create the cube map
		std::unique_ptr<CubeMapFrameBuffer> dynamicCubeMap = std::make_unique<CubeMapFrameBuffer>(dynamicEnvMapWidthHeight, true);
		//Bind it to entity
		entAdv->setEnvironmentMap(dynamicCubeMap->getTexture());
		//Set it up for rendering somewhere
		dynamicEnvMaps.push_back(std::make_tuple(std::move(dynamicCubeMap), entAdv));
	}
	//Use skybox
	else
		entAdv->setEnvironmentMap(this->SkyBox()->getTexture());
}
void BasicScene::disableEnvironmentMap(std::shared_ptr<RenderableAdv> entAdv)
{
	entAdv->setEnvironmentMap(nullptr);
	for (auto a = dynamicEnvMaps.begin(); a != dynamicEnvMaps.end(); ++a)
	{
		if (std::get<1>(*a) == entAdv)
		{
			dynamicEnvMaps.erase(a);
			return;
		}
	}
}
void BasicScene::_render()
{
	//Render to any dynamic environment maps
	for (auto &a : dynamicEnvMaps)
	{
		auto &cubeMap = std::get<0>(a);
		auto &ent = std::get<1>(a);
		if (ent->visible())
		{
			//Set source model to not visible
			ent->visible(false);
			//Override projection matrix
			this->visualisation.setProjectionMat(CubeMapFrameBuffer::getProjecitonMat());
			//For each face of cube map framebuffer
			for (unsigned int i = 0; i < 6; ++i)
			{
				CubeMapFrameBuffer::Face f = CubeMapFrameBuffer::Face(i);
				//Use cube map face
				cubeMap->use(f);
				//Overrides view matrix
				this->visualisation.getCamera()->setViewMat(CubeMapFrameBuffer::getViewMat(f, ent->getLocation()));
				this->visualisation.getCamera()->setSkyBoxViewMat(CubeMapFrameBuffer::getSkyBoxViewMat(f));
				//Update lighting buffer
				lighting->update();
				//Render scene	
				if (this->renderSkyboxState)
					this->skybox->render();
				if (this->renderAxisState)
					this->axis->render();
				render();
			}
			//Update mipmap (I hate this step needing to be manually called)
			cubeMap->updateMipMap();
			//Reset framebuffer (implicitly handled at next render pass)
			//Reset matricies
			this->visualisation.resetProjectionMat();
			this->visualisation.getCamera()->resetViewMats();
			//Update lighting buffer
			lighting->update();
			//Reset source model to visible
			ent->visible(true);
		}
	}
	//Bind back buffer
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glClearColor(0, 0, 0, 1));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	//Reset viewport
	GL_CALL(glViewport(0, 0, visualisation.getWindowWidth(), visualisation.getWindowHeight()));
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