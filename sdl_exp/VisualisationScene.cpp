#include "VisualisationScene.h"
typedef float float4[4];

#define ENV_ROW_COUNT 4
#define ENV_NODE_OFFSET 8
#define ENV_COLOUR_COUNT 2
#define ENV_NODE_SCALE 1.0f

#define AGENT_SCALE 1.0f

VisualisationScene::VisualisationScene(Camera* camera, Shaders* vechShaders, Shaders* envShaders) : camera(camera)
{
	this->camera = camera;
	this->vechShaders = vechShaders;
	this->envShaders = envShaders;
	this->object = new Entity("objects/icosphere.obj", AGENT_SCALE);
	this->mesh256 = new Entity("objects/cube.obj", ENV_NODE_SCALE);
	this->axis = new Axis(5.0);

	// Do some Texture stuff here for now, realted to the numbe of instnaces.
	int agentsPerRoad = 1;
	int agentsPerRow = (ENV_ROW_COUNT ) * agentsPerRoad;
	float agentOffsetPerRoad = ENV_NODE_OFFSET / (agentsPerRoad +1);
	

	this->agentCount = agentsPerRow * agentsPerRow;
	this->agent_texture = new Texture();
	this->agent_position_data_tbo = 0;
	this->agent_position_data_tex = 0;
	this->agent_texture->createTextureBufferObject(&this->agent_position_data_tbo, &this->agent_position_data_tex, this->agentCount);

	// Generate some mock data
	float4 *agentLocationData = (float4*)malloc(this->agentCount*sizeof(float4));
	for (int i = 0; i < this->agentCount / agentsPerRow; ++i){
		for (int j = 0; j < agentsPerRow; ++j){
			int index = (i*agentsPerRow) + j;
			agentLocationData[index][0] = agentOffsetPerRoad + (ENV_NODE_OFFSET * i);
			agentLocationData[index][1] = 0;
			agentLocationData[index][2] =  (ENV_NODE_OFFSET * j);
			agentLocationData[index][3] = i % 3;
		}
	}

	// Bind the mock data?
	glBindBuffer(GL_TEXTURE_BUFFER, this->agent_position_data_tbo);
	glBufferData(GL_TEXTURE_BUFFER, this->agentCount * sizeof(float4), agentLocationData, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, v_count*sizeof(float3) * 2, vertices, GL_DYNAMIC_DRAW);

	// Initialise and generate environment texture
	this->environmentCount = ENV_ROW_COUNT * ENV_ROW_COUNT;
	this->environment_texture = new Texture();
	this->environment_position_data_tbo = 0;
	this->environment_position_data_tex = 0;
	this->environment_texture->createTextureBufferObject(&this->environment_position_data_tbo, &this->environment_position_data_tex, this->environmentCount);

	// Generate some mock data
	float4 *envLocationData = (float4*)malloc(this->environmentCount*sizeof(float4));
	for (int i = 0; i < this->environmentCount; ++i){
		float4 d = { i, 0.0f, 0.0f, 1.0f };
		envLocationData[i][0] = ENV_NODE_OFFSET * (i / ENV_ROW_COUNT);
		envLocationData[i][1] = 0.0f;
		envLocationData[i][2] = ENV_NODE_OFFSET * (i % ENV_ROW_COUNT);
		envLocationData[i][3] = i % ENV_COLOUR_COUNT;
	}
	// Bind the mock data?
	glBindBuffer(GL_TEXTURE_BUFFER, this->environment_position_data_tbo);
	glBufferData(GL_TEXTURE_BUFFER, this->environmentCount * sizeof(float4), envLocationData, GL_STATIC_DRAW);

	// Bind texture data - this probs shouldnt be here.
	

}


VisualisationScene::~VisualisationScene()
{
	delete this->object;
	delete this->axis;
	delete this->agent_texture;
	delete this->environment_texture;
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

	// Use the environment shadersand texture to render the environment
	this->envShaders->useProgram();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, this->environment_position_data_tex);
	glPushMatrix();
		glScaled(ENV_NODE_SCALE, ENV_NODE_SCALE, ENV_NODE_SCALE);
		this->mesh256->renderInstances(this->environmentCount);
	glPopMatrix();

	this->envShaders->clearProgram();
	// Use the vech shaders to render the agents
	this->vechShaders->useProgram();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, this->agent_position_data_tex);
	glPushMatrix();
		glScaled(AGENT_SCALE, AGENT_SCALE, AGENT_SCALE);
		this->object->renderInstances(this->agentCount);
	glPopMatrix();

	

	this->vechShaders->clearProgram();
}
