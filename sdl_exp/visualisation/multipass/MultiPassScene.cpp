#include "MultiPassScene.h"

/*
Sets this scene within the visualisation
@param visualisation The visualisation that is hosting the scene.
*/
MultiPassScene::MultiPassScene(Visualisation &visualisation)
    : Scene(visualisation)
{

}
/*
Registers an entity, so the scene can manage it's modelview and projection matrices and reloads
*/
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
/*
Reloads all registered entities, then calls reload on the subclassed scene
*/
void MultiPassScene::_reload()
{
    printf("Reloading Shaders.\n");
	for (std::vector<std::shared_ptr<Renderable>>::iterator i = entities.begin(); i != entities.end(); i++)
    {
        (*i)->reload();
    }
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

void MultiPassScene::executeRender()
{
    for (auto&& it : rpMap)//byRef
    {
        it.second->executeRender();
    }
}
void MultiPassScene::resize(const int width, const int height)
{
    for (auto&& it : rpMap)//byRef
    {
        it.second->resize(width, height);
    }
}

