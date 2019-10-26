#include "MultiPassScene.h"

MultiPassScene::MultiPassScene(ViewportExt &visualisation)
	: Scene(visualisation)
	, lighting(std::make_shared<LightsBuffer>(visualisation.getCamera()->getViewMatPtr()))
{
    lighting->setProjectionMatPtr(this->visualisation.getProjectionMatPtr());
}
void MultiPassScene::registerEntity(std::shared_ptr<Renderable> ent)
{
    if (ent.get())
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
void MultiPassScene::_reload()
{
    printf("Reloading Shaders.\n");
	for (auto&& it : entities)//byRef
        it->reload();
    reload();
}

std::shared_ptr<RenderPass> MultiPassScene::addPass(int index, std::shared_ptr<RenderPass> rp, bool enabled)
{
    auto&& it = rpMap.find(index);//byRef
    std::shared_ptr<RenderPass> rtn = std::shared_ptr<RenderPass>();
    if (it != rpMap.end())
    {
        rtn = it->second.first;
    }
	rpMap[index] = { rp, enabled };
	rp->resize(visualisation.getWindowWidth(), visualisation.getWindowHeight());
    return rtn;
}

void MultiPassScene::_render()
{
	lighting->update();
    for (auto&& it : rpMap)//byRef
		if(it.second.second)
			it.second.first->executeRender();
}
void MultiPassScene::_resize(const glm::uvec2 &dims)
{
    for (auto&& it : rpMap)//byRef
        it.second.first->resize(dims);
}

void MultiPassScene::setPassActive(std::shared_ptr<RenderPass> rp, bool enabled)
{
	for (auto&& it : rpMap)//byRef
		if (it.second.first == rp)
			it.second.second = enabled;
}
void MultiPassScene::setPassActive(int index, bool enabled)
{
	auto&& it = rpMap.find(index);//byRef
	if (it != rpMap.end())
	{
		it->second.second = enabled;
	}
}

