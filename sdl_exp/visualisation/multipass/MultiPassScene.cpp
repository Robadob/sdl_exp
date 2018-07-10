#include "MultiPassScene.h"

MultiPassScene::MultiPassScene(Visualisation &visualisation)
	: Scene(visualisation)
	, lighting(std::make_shared<LightsBuffer>(visualisation.getCamera()->getViewMatPtr()))
{
	lighting->setProjectionMatPtr(this->visualisation.getProjectionMatPtr());
}
void MultiPassScene::registerEntity(std::shared_ptr<Renderable> ent)
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
void MultiPassScene::enableEnvironmentMap(std::shared_ptr<RenderableAdv> entAdv, const int &passIndex, std::shared_ptr<RenderPass> renderpass, const unsigned int &dynamicEnvMapWidthHeight, const glm::vec3 &originOffset)
{
	assert(dynamicEnvMapWidthHeight > 0);
	//Don't allow duplicate envMaps
	disableEnvironmentMap(entAdv);
	//Create a custom renderpass for the cubemap
	//Create the cube map
	std::shared_ptr<CubeMapFrameBuffer> dynamicCubeMap = std::make_shared<CubeMapFrameBuffer>(dynamicEnvMapWidthHeight, true);
	//Bind it to entity
	entAdv->setEnvironmentMap(dynamicCubeMap->getTexture());
	//Create the render pass
	std::shared_ptr<CubeMapPass> cubeMapPass = std::make_shared<CubeMapPass>(dynamicCubeMap, renderpass, entAdv, visualisation, lighting, originOffset);
	//Set the render pass (so the dynamic pass is updated somewhere
	addPass(passIndex, cubeMapPass);
	//Log it, so we can delete it later
	dynamicEnvMaps.push_back(std::make_tuple(cubeMapPass, entAdv));
}
void MultiPassScene::disableEnvironmentMap(std::shared_ptr<RenderableAdv> entAdv)
{
	entAdv->setEnvironmentMap(nullptr);
	for (auto a = dynamicEnvMaps.begin(); a != dynamicEnvMaps.end(); ++a)
	{
		if (std::get<1>(*a) == entAdv)
		{
			removePass(std::get<0>(*a));
			dynamicEnvMaps.erase(a);
			return;
		}
	}
}
void MultiPassScene::_reload()
{
    printf("Reloading Shaders.\n");
	for (auto&& it : entities)//byRef
        it->reload();
    reload();
}

void MultiPassScene::addPass(const int &index, std::shared_ptr<RenderPass> rp)
{	
	rpMap.insert({index, rp});
}
void MultiPassScene::removePass(std::shared_ptr<RenderPass> rp)
{
	for (auto &&it = rpMap.begin(); it != rpMap.end(); ++it)
		if (it->second == rp)
			rpMap.erase(it);
}
void MultiPassScene::_render()
{
	lighting->update();
	for (auto &&it = rpMap.begin(); it != rpMap.end(); ++it)
        it->second->executeRender();
}
void MultiPassScene::_resize(const int width, const int height)
{
	for (auto &&it = rpMap.begin(); it != rpMap.end(); ++it)
        it->second->resize(width, height);
}
