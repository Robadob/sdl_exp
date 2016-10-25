#include "MultiPassScene.h"

MultiPassScene::MultiPassScene(Visualisation &visualisation)
    : Scene(visualisation)
{

}
void MultiPassScene::registerEntity(std::shared_ptr<Renderable> ent)
{
    if (ent.get())
    {
        //Store value for later
        entities.push_back(ent);
        //Setup matrices
        ent->setModelViewMatPtr(this->visualisation.getCamera());
        ent->setProjectionMatPtr(&this->visualisation);
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

std::shared_ptr<RenderPass> MultiPassScene::addPass(int index, std::shared_ptr<RenderPass> rp)
{
    auto&& it = rpMap.find(index);//byRef
    std::shared_ptr<RenderPass> rtn = std::shared_ptr<RenderPass>();
    if (it != rpMap.end())
    {
        rtn = it->second;
    }
    rpMap[index] = rp;
    return rtn;
}

void MultiPassScene::_render()
{
    for (auto&& it : rpMap)//byRef
        it.second->executeRender();
}
void MultiPassScene::_resize(const int width, const int height)
{
    for (auto&& it : rpMap)//byRef
        it.second->resize(width, height);
}
