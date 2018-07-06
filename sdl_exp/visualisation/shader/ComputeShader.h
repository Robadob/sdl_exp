#ifndef __ComputeShader_h__
#define __ComputeShader_h__
#include "ShaderCore.h"
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>

/**
 * Class representative of a compute shader, it wraps the common bindings found in ShaderCore and adds a custom wrapper
 * to glUseProgram();glDispatchCompute() which can be access via the numerous launch methods()
 * Good intro to compute shaders here: https://www.cg.tuwien.ac.at/courses/Realtime/repetitorium/rtr_rep_2014_ComputeShader.pdf
 * @see ShaderCore
 * @author Rob
 */
class ComputeShader : public ShaderCore
{
public:
	/**
	* Creates a compute shader from a single source file
	* @param path File path to be included in the shader
	* @param defaultLaunchConfig Default launch config (useful if you will be using the same config each launch)
	*/
	ComputeShader(const char * path, glm::uvec3 defaultLaunchConfig = glm::uvec3(0));
	/**
	 * Creates a compute shader from multiple source files
	 * Use this constructor if you wish to constructor a shader from multiple files (e.g. a common functions file)
	 * @param paths Init list of file paths to be included in the shader
	 * @param defaultLaunchConfig Default launch config (useful if you will be using the same config each launch)
	 * @note The file containing main() should be the final item (this is just an assumption)
	 */
	ComputeShader(std::initializer_list<std::string> paths, glm::uvec3 defaultLaunchConfig = glm::uvec3(0));
	/**
	 * Copy constructor
	 */
	ComputeShader(const ComputeShader &other);
	/**
	 * Cleans up shader files vector
	 * Frees program
	 */
	~ComputeShader();
	/**
	* Launches the compute shader according to the provided launch configuration
	* @param workGroupsX The number of work groups in the X axis
	* @param workGroupsY The number of work groups in the Y axis
	* @param workGroupsZ The number of work groups in the Z axis
	* @note Workgroups are not threads, workgroups have their own upto 3D size which must be specified within the shaders code
	*/
	inline void launch(GLuint workGroupsX, GLuint workGroupsY = 1, GLuint workGroupsZ = 1)
	{
		launch(glm::uvec3(workGroupsX, workGroupsY, workGroupsZ));
	}
	/**
	* Launches the compute shader according to the provided launch configuration
	* @param workGroups A 3D workgroup specification
	* @note Workgroups are not threads, workgroups have their own upto 3D size which must be specified within the shaders code
	*/
	inline void launch(glm::uvec3 workGroups)
	{
		lastLaunchConfig = workGroups;
		launch();
	}
	/**
	* Launches the compute shader according to the provided launch configuration
	* @param workGroups A 2D workgroup specification
	* @note Workgroups are not threads, workgroups have their own upto 3D size which must be specified within the shaders code
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
	/**
	* Calls compileShader() with shaderFilePaths
	* @return The return value of the compileShader() call
	*/
	bool _compileShaders(const GLuint t_shaderProgram) override;
	glm::uvec3 lastLaunchConfig; 	
	std::vector<std::string> *shaderFilePaths;
};

#endif //ifndef __ComputeShader_h__