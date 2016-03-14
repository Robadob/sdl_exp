#include "Scene.h"

/*
Binds the ModelView and Projection matrices into the provided shaer
Sets this scene within the visualisation
*/
Scene::Scene(Visualisation &visualisation, Shaders *shaders)
    : visualisation(visualisation)
    , shaders(shaders)
{
    if (this->shaders)
    {
        this->shaders->setModelViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());
        this->shaders->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
    }
    this->visualisation.setScene(this);
}
/*
Calls the destructor
*/
void Scene::kill(){
    delete this;
}