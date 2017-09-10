#include "Sort.h"

Sort::Sort(const glm::vec3 *eye)
	: bitonicShader(std::make_shared<ComputeShader>(BITONIC_NETWORK_PATH))
	, activeBufferName(0)
	, u_eyeLoc(eye)
{
	bitonicShader->addDynamicUniform("hop", &this->u_hop);
	bitonicShader->addDynamicUniform("hop2", &this->u_hop2);
	bitonicShader->addDynamicUniform("itemCount", &this->u_itemCount);
	bitonicShader->addDynamicUniform("periodisation", &this->u_periodisation);
	bitonicShader->addDynamicUniform("direction", &this->u_direction);
	bitonicShader->addDynamicUniform("threadCount", &this->u_threadCount);
	bitonicShader->addDynamicUniform("eye", reinterpret_cast<const GLfloat*>(this->u_eyeLoc), 3);
}
void Sort::sort(const GLuint bufferName, const unsigned int count, const Order order)
{
	if (activeBufferName != bufferName)
	{
		bitonicShader->addBuffer("Items", GL_SHADER_STORAGE_BUFFER, bufferName);
		activeBufferName = bufferName;
	}
	//Bitonic Sequence Generator
	this->u_itemCount = count;
	this->u_direction = -1 * order;//Direction must be inverse throughout sequence for non 2^n populations to work
	const int nextPow2 = static_cast<int>(ceil(log(count) / log(2)));
	for (int stage = 0; stage < (nextPow2 - 1); ++stage)
	{
		this->u_periodisation = 1 << stage;
		for (int pass = stage; pass >= 0; --pass)
		{
			//Update uniforms
			this->u_hop = 1 << pass;
			this->u_hop2 = this->u_hop << 1;
			this->u_threadCount = static_cast<int>(pow(2, nextPow2 - 1));
			//Run Shader
			bitonicShader->launch(static_cast<unsigned int>(ceil((count / 2.0f) / 256.0)));
			//Barrier for memory to finish
			GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
		}
	}
	//Bitonic Merge
	this->u_direction = order;//-1 Dir gives us them in descending order (+1 for asc)
	this->u_periodisation = count;//Count sets 'flip' to 0 for all threads
	for (int i = nextPow2 - 1; i >= 0; i--)
	{
		//Update uniforms
		this->u_hop = 1 << i;
		this->u_hop2 = this->u_hop << 1;
		this->u_threadCount = static_cast<int>(pow(2, nextPow2 - 1));
		//Run Shader
		bitonicShader->launch(static_cast<unsigned int>(ceil((count / 2.0f) / 256.0)));
		//Barrier for memory to finish
		GL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
	}
}
void Sort::reload()
{
	bitonicShader->reload();
}
//Think this was used in testing, not as advanced as shader bitonic network
//void cpuBitonicMerge(int particleCount, int hop, int hop2, glm::vec4 *data, glm::vec3 eye)
//{
//	for (int i = 0; i < 256; ++i)
//	{
//		//Calculate indexes to be Compare&Swap'd
//		int indexA = (i%hop) + ((i / hop)*hop2);
//		int indexB = indexA + hop;
//
//		//Check indexes are in range
//		if (indexB >= particleCount || indexA >= particleCount)
//			return;
//
//		//Read particles
//		int mapA = glm::floatBitsToInt(data[indexA].w);
//		glm::vec3 particleA = glm::vec3(data[mapA]);
//		int mapB = glm::floatBitsToInt(data[indexB].w);
//		glm::vec3 particleB = glm::vec3(data[mapB]);
//
//		//printf("%d: %d<->%d\n", i, indexA, indexB);
//		//Compare (Bring furthest to front)
//		if (length(particleB - eye)>length(particleA - eye))
//		{
//			//and Swap
//			data[indexA].w = glm::intBitsToFloat(mapB);
//			data[indexB].w = glm::intBitsToFloat(mapA);
//		}
//	}
//}