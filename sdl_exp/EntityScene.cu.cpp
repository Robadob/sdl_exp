#include "EntityScene.h"
#include <glm/gtc/type_ptr.hpp>
#include "visualisation/ComputeShader.h"
#include <ctime>
#define PARTICLE_COUNT 100
/*
Constructor, modify this to change what happens
*/
EntityScene::EntityScene(Visualisation &visualisation)
    : Scene(visualisation)
    , deerModel(new Entity(Stock::Models::DEER, 10.0f, Stock::Shaders::TEXTURE))
    , colorModel(new Entity(Stock::Models::ROTHWELL, 45.0f, Stock::Shaders::COLOR))
    , tick(0.0f)
    , polarity(-1)
    , instancedSphere(new Entity(Stock::Models::ICOSPHERE, 1.0f, Stock::Shaders::INSTANCED))
#ifdef __CUDACC__
	, cuTexBuf(mallocGLInteropTextureBuffer<float>(PARTICLE_COUNT, 3))
    , texBuf("_texBuf", cuTexBuf, true)
#else
	, texBuf("_texBuf", PARTICLE_COUNT, 3)
#endif
	, billboardShaders(new Shaders(Stock::Shaders::BILLBOARD))
	, particleSort(nullptr)
{
    registerEntity(deerModel);
    registerEntity(colorModel);
    registerEntity(instancedSphere);
    this->visualisation.setSkybox(true);
    this->visualisation.setWindowTitle("Entity Render Sample");
    this->visualisation.setRenderAxis(true); 
    srand((unsigned int)time(0));
    this->colorModel->setRotation(glm::vec4(1.0, 0.0, 0.0, -90));
    this->colorModel->setCullFace(false);
    this->deerModel->flipVertexOrder();
#ifdef __CUDACC__
    cuInit();
#else
	float *tempData = (float*)malloc(sizeof(float) * 3 * PARTICLE_COUNT);
	for (int i = 0; i < PARTICLE_COUNT; i++)
    {
		tempData[(i * 3) + 0] = PARTICLE_COUNT * (float)sin(i*3.6);
        tempData[(i * 3) + 1] = -50.0f;
		tempData[(i * 3) + 2] = PARTICLE_COUNT * (float)cos(i*3.6);
    }
    texBuf.setData(tempData);
    free(tempData);
#endif
    texBuf.bindToShader(this->instancedSphere->getShaders().get());
    this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
   // initParticles();

	//assimpTest = std::make_shared<Model>("C:\\Users\\rob\\Desktop\\assimp models\\X\\dwarf.x");
	//assimpTest = std::make_shared<Model>("C:\\Users\\rob\\Desktop\\assimp models\\B3D\\dwarf2.b3d");
	//assimpTest = std::make_shared<Model>("C:\\Users\\rob\\Desktop\\assimp models\\X\\Testwuson.X");
	assimpTest = std::make_shared<Model>("C:\\Users\\rob\\Desktop\\assimp models\\X\\BCN_Epileptic.X");
	flatShader = std::make_shared<Shaders>(Stock::Shaders::FLAT);
	flatShader->setModelViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());
	flatShader->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
	flatShader->setColor(glm::vec3(1));
}
/*
Called once per frame when Scene animation calls should be 
@param Milliseconds since last time this method was called
*/
void EntityScene::update(unsigned int frameTime)
{
    this->tick += this->polarity*((frameTime*60)/1000.0f)*0.01f;
    this->tick = (float)fmod(this->tick,360);


    this->particleTick += this->polarity*(0.02f);
    this->deerModel->setRotation(glm::vec4(0.0, 1.0, 0.0, this->tick*-100));
    this->deerModel->setLocation(glm::vec3(50 * sin(this->tick), 0, 50 * cos(this->tick)));
#ifdef __CUDACC__
    cuUpdate();
#endif
}
/*
Called once per frame when Scene render calls should be executed
*/
void EntityScene::render()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	assimpTest->render(flatShader);
	glEnable(GL_CULL_FACE);

    //colorModel->render();
    //deerModel->render();
    //this->instancedSphere->renderInstances(100);
	//renderParticles();
}
/*
Called when the user requests a reload
*/
void EntityScene::reload()
{
    this->instancedSphere->setColor(glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX));
    this->billboardShaders->reload();
	flatShader->reload();
	assimpTest->reload();
	flatShader->setColor(glm::vec3(1));
}

bool EntityScene::keypress(SDL_Keycode keycode, int x, int y)
{
    switch (keycode)
    {
    case SDLK_p:
        this->polarity = ++this->polarity>1 ? -1 : this->polarity;
        break;
    case SDLK_HASH:
        this->colorModel->exportModel();
        this->deerModel->exportModel();
    default:
        //Only permit the keycode to be processed if we haven't handled personally
        return true;
    }
    return false;
}
void EntityScene::initParticles()
{
    this->billboardShaders->setModelViewMatPtr(this->visualisation.getCamera()->getViewMatPtr());//&particleTransform);// 
    this->billboardShaders->setProjectionMatPtr(this->visualisation.getFrustrumPtr());
    //Init vbo's
    unsigned int bufferSize = 0;
    bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
    bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
    particleData = malloc(bufferSize);
    //Setup tex coords
    glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(particleData)+4));
    texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
    texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
    texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
    texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
    //Initalise buffer
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, particleData, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Setup vertices
    glm::vec3 *topLeft = static_cast<glm::vec3*>(particleData);
    glm::vec3 *bottomLeft = topLeft + 1;
    glm::vec3 *topRight = bottomLeft + 1;
    glm::vec3 *bottomRight = topRight + 1;
    const float size = 1;
    *topLeft = glm::vec3(-size, -size, 0);
    *bottomLeft = glm::vec3(size, -size, 0);
    *topRight = glm::vec3(-size, size, 0);
    *bottomRight = glm::vec3(size, size, 0);
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), particleData));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    //Link Vertex Attributes
    Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
    pos.vbo = vbo;
    pos.count = 4;
    pos.data = particleData;
    pos.offset = 0;
    pos.stride = 0;
    this->billboardShaders->setPositionsAttributeDetail(pos);
    Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
    texCo.vbo = vbo;
    texCo.count = 4;
    texCo.data = texCoords;
    texCo.offset = 4 * sizeof(glm::vec3);
    texCo.stride = 0;
    this->billboardShaders->setTexCoordsAttributeDetail(texCo);
    //Setup faces
    GL_CALL(glGenBuffers(1, &fvbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    const int faces[] = { 0, 1, 2, 3 };
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(int), &faces, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    //Init uniforms
    this->particleTick = 0;
    this->billboardShaders->addDynamicUniform("_tick", &this->particleTick);
	//Bind uniforms
	this->billboardShaders->addDynamicUniform("_up", reinterpret_cast<const GLfloat *>(this->visualisation.getCamera()->getUpPtr()), 3);
	this->billboardShaders->addDynamicUniform("_right", reinterpret_cast<const GLfloat *>(this->visualisation.getCamera()->getRightPtr()), 3);
	//Compute Shader
	particleSort = new ComputeShader("../shaders/bitonicNetwork.comp");
	float *tempData = (float*)malloc(sizeof(float) * 4 * PARTICLE_COUNT);
	int j = 0;
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		tempData[(i * 4) + 0] = 35 * (float)sin(i*(6.28319f / (PARTICLE_COUNT)));
		tempData[(i * 4) + 1] = 0;// -50.0f;
		tempData[(i * 4) + 2] = 35 * (float)cos(i*(6.28319f / (PARTICLE_COUNT)));
		tempData[(i * 4) + 3] = *reinterpret_cast<float*>(&i);//Store int bytes as float
		j += i;
	}
	
	//ssbo;//shader storage buffer object
	GL_CALL(glGenBuffers(1, &ssbo));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo));
	GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT*sizeof(glm::vec4), tempData, GL_DYNAMIC_DRAW));
	GLuint block_index = 0;
	block_index = glGetProgramResourceIndex(particleSort->getProgram(), GL_SHADER_STORAGE_BLOCK, "Particles");
	GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, block_index, ssbo));
	//Also bind buffer to render shader

	
	block_index = glGetProgramResourceIndex(billboardShaders->getProgram(), GL_SHADER_STORAGE_BLOCK, "Particles");
	GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, block_index, ssbo));

	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

	nextPow2 = (int)ceil(log(PARTICLE_COUNT) / log(2));

	//Find compute shader uniforms
	particleCtLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "particleCount"));
	threadCtLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "threadCount"));
	hopLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "hop"));
	hop2Loc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "hop2"));
	eyeLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "eye"));
	periodisationLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "periodisation"));
	directionLoc = GL_CALL(glGetUniformLocation(particleSort->getProgram(), "direction"));
	if (particleCtLoc == -1 || hopLoc == -1 || hop2Loc == -1 || eyeLoc == -1 || periodisationLoc == -1 || directionLoc == -1) printf("err compute shader uniform missing.\n");

	free(tempData);
}
void cpuBitonicMerge(int particleCount, int hop, int hop2, glm::vec4 *data, glm::vec3 eye)
{
	for (int i = 0; i < 256;++i)
	{
		//Calculate indexes to be Compare&Swap'd
		int indexA = (i%hop) + ((i / hop)*hop2);
		int indexB = indexA + hop;

		//Check indexes are in range
		if (indexB >= particleCount || indexA >= particleCount)
			return;

		//Read particles
		int mapA = glm::floatBitsToInt(data[indexA].w);
		glm::vec3 particleA = glm::vec3(data[mapA]);
		int mapB = glm::floatBitsToInt(data[indexB].w);
		glm::vec3 particleB = glm::vec3(data[mapB]);

		//printf("%d: %d<->%d\n", i, indexA, indexB);
		//Compare (Bring furthest to front)
		if (length(particleB-eye)>length(particleA-eye))
		{
			//and Swap
			data[indexA].w = glm::intBitsToFloat(mapB);
			data[indexB].w = glm::intBitsToFloat(mapA);
		}
	}
}
void EntityScene::renderParticles()
{
	GL_CALL(glUseProgram(particleSort->getProgram()));
	//Bitonic Sequence Generator
	for (int stage = 0; stage < (nextPow2 - 1); ++stage)
	{
		int periodisation = 1 << stage;
		for (int pass = stage; pass >= 0; --pass)
		{
			int hop = 1 << pass;
			int hop2 = hop << 1;
			//Update uniforms
			int pc = PARTICLE_COUNT;
			GL_CALL(glUniform1iv(particleCtLoc, 1, (GLint *)&pc));
			int tc = (int)pow(2, nextPow2 - 1);
			GL_CALL(glUniform1iv(threadCtLoc, 1, (GLint *)&tc));
			GL_CALL(glUniform1iv(hopLoc, 1, (GLint *)&hop));
			GL_CALL(glUniform1iv(hop2Loc, 1, (GLint *)&hop2));
			GL_CALL(glUniform1iv(periodisationLoc, 1, (GLint *)&periodisation));
			int dir = 1;//Direction must be inverse throughout sequence for non 2^n populations to work
			GL_CALL(glUniform1iv(directionLoc, 1, (GLint *)&dir));
			GL_CALL(glUniform3fv(eyeLoc, 1, (GLfloat *)visualisation.getCamera()->getEyePtr()));
			particleSort->launch((unsigned int)ceil((PARTICLE_COUNT / 2.0f) / 256.0));
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}
	//Bitonic Merge
	for (int i = nextPow2 - 1; i >= 0; i--)
	{
		int hop = 1 << i;
		int hop2 = hop << 1;

		//Update uniforms
		int pc = PARTICLE_COUNT;
		GL_CALL(glUniform1iv(particleCtLoc, 1, (GLint *)&pc));
		int tc = (int)pow(2, nextPow2 - 1);
		GL_CALL(glUniform1iv(threadCtLoc, 1, (GLint *)&tc));
		GL_CALL(glUniform1iv(hopLoc, 1, (GLint *)&hop));
		GL_CALL(glUniform1iv(hop2Loc, 1, (GLint *)&hop2));
		GL_CALL(glUniform3fv(eyeLoc, 1, (GLfloat *)visualisation.getCamera()->getEyePtr()));
		GL_CALL(glUniform1iv(periodisationLoc, 1, (GLint *)&pc));//The particle count sets direction to 0 throughout
		int dir = -1;//-1 Dir gives us them in descending order (+1 for asc)
		GL_CALL(glUniform1iv(directionLoc, 1, (GLint *)&dir));
		particleSort->launch((unsigned int)ceil((PARTICLE_COUNT / 2.0f) / 256.0));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	//Render them
    GL_CALL(glEnable(GL_BLEND));
	//GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    //GL_CALL(glDisable(GL_DEPTH_TEST));
    //Use Shader
    billboardShaders->useProgram();
    //Render quad
    glDisable(GL_CULL_FACE);
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    glPushMatrix();
	GL_CALL(glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr, PARTICLE_COUNT));
    glPopMatrix();

    //Unload shader
    glEnable(GL_CULL_FACE);
    billboardShaders->clearProgram();

    GL_CALL(glDisable(GL_BLEND));
    //GL_CALL(glEnable(GL_DEPTH_TEST));
}