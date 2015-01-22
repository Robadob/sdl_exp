#include "VisualisationScene.h"

VisualisationScene::VisualisationScene(Camera* camera) : camera(camera)
{
	this->camera = camera;
	this->object = new Entity("objects/cube.obj", 1.0f);
	this->axis = new Axis(5.0);
}


VisualisationScene::~VisualisationScene()
{
	delete this->object;
	delete this->axis;
}


void VisualisationScene::update(){
	// Do nothing.
}

void VisualisationScene::render(){
	glClearColor(0, 0, 0, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	this->camera->view();
	// Place lighting here, before any objects

	// Objects

	this->axis->render();

	glPushMatrix();
		this->object->render();
	glPopMatrix();
}
