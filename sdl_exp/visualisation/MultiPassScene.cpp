#include "Scene.h"

/*
Sets this scene within the visualisation
@param visualisation The visualisation that is hosting the scene.
*/
Scene::Scene(Visualisation &visualisation)
    : visualisation(visualisation)
{
    this->visualisation.setScene(this);
}
/*
Calls the destructor
*/
void Scene::kill(){
    delete this;
}
/*
Registers an entity, so the scene can manage it's modelview and projection matrices and reloads
*/
void Scene::registerEntity(std::shared_ptr<Entity> ent)
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
void Scene::_reload()
{
    printf("Reloading Shaders.\n");
    for (std::vector<std::shared_ptr<Entity>>::iterator i = entities.begin(); i != entities.end(); i++)
    {
        (*i)->reload();
    }
    reload();
}

std::shared_ptr<RenderPass> Scene::addPass(int index, std::shared_ptr<RenderPass> rp)
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

void Scene::executeRender()
{
    for (auto&& it : rpMap)//byRef
    {
        it.second->executeRender();
    }
}
void Scene::resize(const int width, const int height)
{
    for (auto&& it : rpMap)//byRef
    {
        it.second->resize(width, height);
    }
}

