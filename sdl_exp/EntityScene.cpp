#include "EntityScene.h"
/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
    : Scene(visualisation)
    , icosphere(new Entity(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
    , colorModel(new Entity(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
    , tick(0.0f)
{
    registerEntity(icosphere);
    registerEntity(colorModel);
    this->visualisation.setSkybox(true);
    this->visualisation.setWindowTitle("Entity Render Sample");
    this->visualisation.setRenderAxis(true); 
    srand((unsigned int)time(0));
    //this->icosphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
    this->colorModel->setColor(glm::vec3(0.0, 1.0, 1.0));
    this->colorModel->setRotation(glm::vec4(1.0, 0.0, 0.0, -90));
    this->colorModel->setCullFace(false);
    //this->colorModel->exportModel();
    this->icosphere->flipVertexOrder();
}
/*
Called once per frame when Scene animation calls should be executed
*/
void EntityScene::update()
{
    this->tick+=0.01f;
    this->tick = (float)fmod(this->tick,360);
    this->icosphere->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick*-100));
    this->icosphere->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
    colorModel->render();
    icosphere->render();
}
/*
Called when the user requests a reload
*/
void EntityScene::reload()
{
    //this->icosphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
}

