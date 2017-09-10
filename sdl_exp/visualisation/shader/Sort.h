#ifndef __Sort_h__
#define __Sort_h__
#include "ComputeShader.h"
#include <memory>

/**
 * Provides convenient compute shader Sort functionality
 * Currently specialised to sorting particles back to front
 * @todo In future hope to generalise this to 1/2/3 dimensional items and custom conditions via templated shaders or similar
 */
class Sort : public Reloadable
{
	enum Order{ ASCENDING = +1, DESCENDING = -1 };
	const char *BITONIC_NETWORK_PATH = "../shaders/bitonicNetwork.comp";
public:
	/**
	 * Builds the compute shader and attaches the dynamic uniforms
	 */
	Sort(const glm::vec3 *eye);
	/**
	 * Performs a sort on the provided array
	 * @param bufferName GLName of a GL_SHADER_STORAGE_BUFFER containing data to be sorted
	 * @param count The number of items to be sorted
	 * @param order The sort direction
	 */
	void sort(const GLuint bufferName, const unsigned int count, const Order order = DESCENDING);
	void reload() override;
private:
	std::shared_ptr<ComputeShader> bitonicShader;
	GLuint activeBufferName;
	//These are mapped to the shader uniforms
	int u_hop, u_hop2, u_itemCount, u_periodisation, u_direction, u_threadCount;
	const glm::vec3 *u_eyeLoc;
};

#endif //__Sort_h__