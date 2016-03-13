#include "VisualisationScene.h"
#include "Visualisation.h"

VisualisationScene::VisualisationScene(Visualisation* vis)
    : vis(vis)
    , shader(0)
    , ent(0)
{
    this->generate();
}
VisualisationScene::~VisualisationScene()
{
    delete ent;
    delete shader;
}
/*
Generate any scene content
*/
void VisualisationScene::generate(){
    shader = new Shaders("../shaders/flat.v", "../shaders/flat.f");
    shader->setModelViewMatPtr(vis->getCamera()->getViewMatPtr());
    shader->setProjectionMatPtr(vis->getFrustrumPtr());
    ent = new Entity("../models/icosphere.obj", 1.0f, shader);
}
/*
Update any scene animations here
*/
void VisualisationScene::update(){

}
/*
Reload any shaders/models here
*/
void VisualisationScene::reload()
{
    shader->reloadShaders();
}
/*
Perform any render calls here
*/
void VisualisationScene::render(glm::mat4 projection)
{
    ent->render();
}
