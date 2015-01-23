#include "VisualisationScene.h"

VisualisationScene::VisualisationScene(Camera* camera) : camera(camera)
{
	this->camera = camera;
	this->object = new Entity("objects/cube1.obj", 1.0f);

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
	
	//@todo temp lighting
	glEnable(GL_LIGHT0);
	float lightPosition[3] = {0, 10, 0};
	float amb[3] = { 0.1f, 0.1f, 0.1f };
	float white[3] = { 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, white);


	// Objects

	//this->axis->render();

	glPushMatrix();
		this->object->render();
	glPopMatrix();
}
