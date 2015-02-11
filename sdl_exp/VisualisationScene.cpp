#include "VisualisationScene.h"
typedef float float4[4];

VisualisationScene::VisualisationScene(Camera* camera, Shaders* shaders) : camera(camera), shaders(shaders)
{
	this->camera = camera;
	this->shaders = shaders;
	this->object = new Entity("objects/icosphere.obj", 1.0f);
	//this->mesh256 = new Entity("objects/mesh256.obj", 1.0f);
	this->axis = new Axis(5.0);

	// Do some Texture stuff here for now, realted to the numbe of instnaces.
	this->agentCount = 64;
	this->texture = new Texture();
	this->agent_position_data_tbo = 0;
	this->agent_position_data_tex = 0;

	this->texture->createTextureBufferObject(&this->agent_position_data_tbo, &this->agent_position_data_tex, this->agentCount);

	// Generate some mock data
	float4 *locationData = (float4*)malloc(this->agentCount*sizeof(float4));
	for (int i = 0; i < this->agentCount; ++i){
		float4 d = { i, 0.0f, 0.0f, 1.0f };
		locationData[i][0] = 1.5 * (i / 8);
		locationData[i][1] = 0.0f;
		locationData[i][2] = 1.5 *(i % 8);
		locationData[i][3] = i % 3;
	}


	// Bind the mock data?
	glBindBuffer(GL_TEXTURE_BUFFER, this->agent_position_data_tbo);
	glBufferData(GL_TEXTURE_BUFFER, this->agentCount * sizeof(float4), locationData , GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, v_count*sizeof(float3) * 2, vertices, GL_DYNAMIC_DRAW);


	// Bind texture data - this probs shouldnt be here.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, this->agent_position_data_tex);

}


VisualisationScene::~VisualisationScene()
{
	delete this->object;
	delete this->axis;
	delete this->texture;
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
	glPushMatrix();
	//	glRotatef(this->tick, 0, 1, 0);

		glEnable(GL_LIGHT0);
		float lightPosition[4] = {0, 10, 0, 1};
		float amb[4] = { 0.1f, 0.1f, 0.1f, 1};
		float white[4] = { 1, 1, 1, 1 };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT0, GL_SPECULAR, white);

		// Spotlight stuff
		float angle = 10.0f;
		float direction[4] = { 0.0f, -1.0f, 0.0f, 0};
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
	glPopMatrix();
	// Objects

	//this->axis->render();
	

	glPushMatrix();
		//glTranslated(0, -5, 0);
		glScaled(0.5, 0.5, 0.5);
		//this->object->render();
		this->object->renderInstances(this->agentCount);
	glPopMatrix();
	glPushMatrix();
		glTranslated(0, -5, 0);
		glScaled(25, 1.0, 25);
		//this->mesh256->render();
	glPopMatrix();

	this->shaders->clearProgram();
}
