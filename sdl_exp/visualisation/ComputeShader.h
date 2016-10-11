#ifndef __ComputeShader_h__
#define __ComputeShader_h__
/*
 * https://www.cg.tuwien.ac.at/courses/Realtime/repetitorium/rtr_rep_2014_ComputeShader.pdf
 */
#include "ShaderCore.h"
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>

class ComputeShader : public ShaderCore
{
public:
	/**
	 * Creates a compute shader
	 * @param paths Init list of file paths to be included in the shader
	 * @param defaultLaunchConfig Default launch config (useful if you will be using the same config each launch)
	 */
	ComputeShader(std::initializer_list<const char *> paths, glm::uvec3 defaultLaunchConfig = glm::uvec3(0));
	/**
	* Launches the compute shader according to the provided launch configuration
	*/
	inline void launch(GLuint workGroupsX, GLuint workGroupsY = 1, GLuint workGroupsZ = 1)
	{
		launch(glm::uvec3(workGroupsX, workGroupsY, workGroupsZ));
	}
	/**
	* Launches the compute shader according to the provided launch configuration
	*/
	inline void launch(glm::uvec3 workGroups)
	{
		lastLaunchConfig = workGroups;
		launch();
	}
	/**
	* Launches the compute shader according to the provided launch configuration
	*/
	inline void launch(glm::uvec2 workGroups)
	{
		launch(glm::uvec3(workGroups, 1));
	}
	/**
	 * Launches the compute shader according to the previous launch configuration
	 */
	void launch();
	/**
	 Returns the maximum launch dimensions in each axis
	 @return Probably (1536,1024,64)
	 @see GL_MAX_COMPUTE_WORK_GROUP_SIZE
	 */
	static glm::uvec3 getMaxWorkGroupDims();
	/**
	 Returns the maximum number of threads in each work group
	 @return Probably 1536
	 @see GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS
	 */
	static unsigned int getMaxThreadsPerWorkGroup();
	/**
	 * Shouldn't really need to call this unless manually attatching items to the shader before launch
	 */
	using ShaderCore::useProgram;
private:
	bool _compileShaders(const GLuint t_shaderProgram) override;
	glm::uvec3 lastLaunchConfig; 	
	std::initializer_list<const char *> shaderFilePaths;
};

#endif