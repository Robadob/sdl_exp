#include "VisualisationScene.h"

VisualisationScene::VisualisationScene(Camera* camera)
    : camera(camera)
{
    this->generate();
}
VisualisationScene::~VisualisationScene()
{

}
/*
Generate any scene content
*/
void VisualisationScene::generate(){

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

}
/*
Perform any render calls here
*/
void VisualisationScene::render(glm::mat4 projection)
{

}
