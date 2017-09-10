#include "ParticleFX.h"

ParticleFX::ParticleFX(const Camera *camera)
	: particleSort(std::make_unique<Sort>(camera->getEyePtr()))
	, billboardShaders(std::make_unique<Shaders>(Stock::Shaders::BILLBOARD))
	, particleLocationBuf(0)
	, PARTICLE_COUNT(127)
	, billboardData(nullptr)
	, billboardVBO(0)
	, billboardFVBO(0)
{
	//Setup our billboard
	this->billboardShaders->addDynamicUniform("_up", reinterpret_cast<const GLfloat *>(camera->getUpPtr()), 3);
	this->billboardShaders->addDynamicUniform("_right", reinterpret_cast<const GLfloat *>(camera->getRightPtr()), 3);
	unsigned int bufferSize = 0;
	bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
	bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
	billboardData = malloc(bufferSize);
	//Setup tex coords
	glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(billboardData)+4));
	texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
	texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
	texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
	texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
	//Initalise buffer
	GL_CALL(glGenBuffers(1, &billboardVBO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, billboardVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, billboardData, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Setup vertices
	glm::vec3 *topLeft = static_cast<glm::vec3*>(billboardData);
	glm::vec3 *bottomLeft = topLeft + 1;
	glm::vec3 *topRight = bottomLeft + 1;
	glm::vec3 *bottomRight = topRight + 1;
	const float size = 1;
	*topLeft = glm::vec3(-size, -size, 0);
	*bottomLeft = glm::vec3(size, -size, 0);
	*topRight = glm::vec3(-size, size, 0);
	*bottomRight = glm::vec3(size, size, 0);
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, billboardVBO));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), billboardData));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Link Vertex Attributes
	Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
	pos.vbo = billboardVBO;
	pos.count = 4;
	pos.data = billboardData;
	pos.offset = 0;
	pos.stride = 0;
	this->billboardShaders->setPositionsAttributeDetail(pos);
	Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
	texCo.vbo = billboardVBO;
	texCo.count = 4;
	texCo.data = texCoords;
	texCo.offset = 4 * sizeof(glm::vec3);
	texCo.stride = 0;
	this->billboardShaders->setTexCoordsAttributeDetail(texCo);
	//Setup faces
	GL_CALL(glGenBuffers(1, &billboardFVBO));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardFVBO));
	const int faces[] = { 0, 1, 2, 3 };
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(int), &faces, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	//Allocate particle data a buffer
	float *tempData = static_cast<float*>(malloc(sizeof(float) * 4 * PARTICLE_COUNT));
	int j = 0;
	for (int i = 0; i < static_cast<int>(PARTICLE_COUNT); i++)
	{
		tempData[(i * 4) + 0] = 35 * static_cast<float>(sin(i * (6.28319f / (PARTICLE_COUNT))));
		tempData[(i * 4) + 1] = 0;// -50.0f;
		tempData[(i * 4) + 2] = 35 * static_cast<float>(cos(i * (6.28319f / (PARTICLE_COUNT))));
		tempData[(i * 4) + 3] = *reinterpret_cast<float*>(&i);//Store int bytes as float
		j += i;
	}
	//ssbo;//shader storage buffer object
	GL_CALL(glGenBuffers(1, &particleLocationBuf));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleLocationBuf));
	GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT*sizeof(glm::vec4), tempData, GL_DYNAMIC_DRAW));
	free(tempData);
	//Bind buffer to billboardShader (for rendering)
	billboardShaders->addBuffer("Particles", GL_SHADER_STORAGE_BUFFER, particleLocationBuf);
}
ParticleFX::~ParticleFX()
{
	if (particleLocationBuf)
		GL_CALL(glDeleteBuffers(1, &particleLocationBuf));
	if (billboardData)
		free(billboardData);
	if (billboardVBO)
		GL_CALL(glDeleteBuffers(1, &billboardVBO));
	if (billboardFVBO)
		GL_CALL(glDeleteBuffers(1, &billboardFVBO));
}
void ParticleFX::render() const
{
	//Do particle motion

	//Sort particles back to front
	this->particleSort->sort(particleLocationBuf, PARTICLE_COUNT);
	//Render them
	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glDisable(GL_CULL_FACE));
	//GL_CALL(glDisable(GL_DEPTH_TEST));
	//GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	//Use Shader & Render Quad
	this->billboardShaders->useProgram();
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, billboardFVBO));
	GL_CALL(glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr, PARTICLE_COUNT));
	//Unload shader
	glEnable(GL_CULL_FACE);
	GL_CALL(glDisable(GL_BLEND));
	//GL_CALL(glEnable(GL_DEPTH_TEST));
	this->billboardShaders->clearProgram();
}

void ParticleFX::reload()
{
	this->particleSort->reload();
	this->billboardShaders->reload();
}
void ParticleFX::setViewMatPtr(glm::mat4 const *viewMat)
{
	this->billboardShaders->setViewMatPtr(viewMat);
}
void ParticleFX::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
	this->billboardShaders->setProjectionMatPtr(projectionMat);
}