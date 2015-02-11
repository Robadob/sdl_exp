#include "VisualisationScene.h"

VisualisationScene::VisualisationScene(Camera* camera, Shaders* shaders) : camera(camera), shaders(shaders)
{
	this->camera = camera;
	this->shaders = shaders;
	this->object = new Entity("objects/cube.obj", 1.0f);
	this->mesh256 = new Entity("objects/mesh256.obj", 1.0f);


	this->axis = new Axis(5.0);
}


VisualisationScene::~VisualisationScene()
{
	delete this->object;
	delete this->axis;
}


void VisualisationScene::update(){
	// Do nothing.
	/*++this->tick;
	if (tick >= 360){
		this->tick = 0;
	}*/
}

void VisualisationScene::render(){
	glClearColor(0, 0, 0, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	this->camera->view();
	this->shaders->useProgram();

	
	// Place lighting here, before any objects
	
	//@todo temp lighting
	//glPushMatrix();
	//	glRotatef(this->tick, 0, 1, 0);

	//	glEnable(GL_LIGHT0);
	//	float lightPosition[4] = {0, 10, 10, 1};
	//	float amb[4] = { 0.1f, 0.1f, 0.1f, 1};
	//	float white[4] = { 1, 1, 1, 1 };
	//	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	//	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	//	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	//	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	//	// Spotlight stuff
	//	float angle = 10.0f;
	//	float direction[4] = { 0.0f, -1.0f, -1.0f, 0};
	//	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
	//	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
	//glPopMatrix();
	// Objects

	//this->axis->render();

	glPushMatrix();
		//glTranslated(0, -5, 0);
		glScaled(0.5, 0.5, 0.5);
		//this->object->render();
		this->object->renderInstances(200);
	glPopMatrix();
	glPushMatrix();
		glTranslated(0, -5, 0);
		glScaled(25, 1.0, 25);
		//this->mesh256->render();
	glPopMatrix();

	this->shaders->clearProgram();
}
