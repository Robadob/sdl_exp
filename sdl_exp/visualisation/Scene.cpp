#include "Scene.h"

/*
Binds the ModelView and Projection matrices into the provided shaer
Sets this scene within the visualisation
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
Registers an entity, so the scene can manage it's matrices and shader reloads
*/
void Scene::registerEntity(std::shared_ptr<Entity> ent)
{
    //Store value for later
    entities.push_back(ent);
    //Setup matrices
    ent->getShaders()->setModelViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());
    ent->getShaders()->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
}
void Scene::_reload()
{
    printf("Reloading Shaders.");
    for (std::vector<std::shared_ptr<Entity>>::iterator i = entities.begin(); i != entities.end(); i++)
    {
        (*i)->getShaders()->reload(true);
    }
    reload();
}