#ifndef __ComputeShader_h__
#define __ComputeShader_h__
/*
 * https://www.cg.tuwien.ac.at/courses/Realtime/repetitorium/rtr_rep_2014_ComputeShader.pdf
 */
#include "ShaderCore.h"
#include "glm/glm.hpp"

class ComputeShader : private ShaderCore
{
	struct ShaderDetail
	{
		ShaderDetail(const char *shaderPath)
			:shaderPath(shaderPath)
			, shaderId(-1)
		{}
		const char *shaderPath;
		int shaderId;
	};
public:
	ComputeShader();
	int addShader(const char* shaderPath);
	int getProgram() const { return programId; };

	bool reload();
	inline void launch(GLuint workGroupsX, GLuint workGroupsY = 1, GLuint workGroupsZ = 1)
	{
		launch(glm::uvec3(workGroupsX, workGroupsY, workGroupsZ));
	}
	inline void launch(glm::uvec3 workGroups)
	{
		lastLaunchConfig = workGroups;
		launch();
	}
	inline void launch(glm::uvec2 workGroups)
	{
		launch(glm::uvec3(workGroups, 1));
	}
	void launch();
	static glm::uvec3 getMaxWorkGroupDims();
	static unsigned int getMaxThreadsPerWorkGroup();
private:
	std::vector<ShaderDetail> shaders;
	int programId;
	glm::uvec3 lastLaunchConfig;
};

#endif