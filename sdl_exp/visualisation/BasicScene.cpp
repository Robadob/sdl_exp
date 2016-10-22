#include "BasicScene.h"
/*
Toggles whether the skybox should be used or not
@param state The desired skybox state
*/
void BasicScene::setSkybox(bool state){
    if (state&&!this->skybox)
    {
        this->skybox = new Skybox();
        this->skybox->setModelViewMatPtr(this->visualisation.getCamera());
        this->skybox->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
        this->skybox->setYOffset(-1.0f);
    }
    else if (!state&&this->skybox)
    {
        delete this->skybox;
        this->skybox = nullptr;
    }
}
/*
Toggles whether the axis should be rendered or not
@param state The desired axis rendering state
*/
void BasicScene::setRenderAxis(bool state){
    this->renderAxisState = state;
}
/*
Provides a simple default lighting configuration located at the camera using the old fixed function pipeline methods
*/
void BasicScene::defaultLighting(){
    glEnable(GL_LIGHT0);
    glm::vec3 eye = this->visualisation.getCamera()->getEye();
    float lightPosition[4] = { eye.x, eye.y, eye.z, 1 };
    float amb[4] = { 0.8f, 0.8f, 0.8f, 1 };
    float diffuse[4] = { 0.2f, 0.2f, 0.2f, 1 };
    float white[4] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    // Spotlight stuff
    //float angle = 180.0f;
    //glm::vec3 look = this->camera.getLook();
    // float direction[4] = { look.x, look.y, look.z, 0 };
    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
}