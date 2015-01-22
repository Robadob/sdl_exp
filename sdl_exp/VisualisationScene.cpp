#include "VisualisationScene.h"

#include "Cube.h"
#include "Axis.h"


VisualisationScene::VisualisationScene(Camera* camera) : camera(camera)
{
	this->camera = camera;
}


VisualisationScene::~VisualisationScene()
{
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
	Cube c1 = Cube();
	Cube c2 = Cube(2);

	Axis axis = Axis(5.0);

	axis.render();

	c1.renderVBO();
	glPushMatrix();
	glTranslated(-4, 0, 4);
	c2.render();
	glPopMatrix();

	/*glPushMatrix();
	glTranslated(10, 0, 10);
	int N = 10;
	for (int i = 0; i < N; ++i){
		for (int j = 0; j < N; ++j){
			for (int k = 0; k < N; ++k){
				glPushMatrix();
					glTranslated(1.1*i, 1.1*j, 1.1*k);
					c1.render();
				glPopMatrix();
			}
		}
	}
	glPopMatrix();
	*/
}
