#include "ComputeShader.h"

ComputeShader::ComputeShader(const char *path, glm::uvec3 defaultLaunchConfig)
	: ComputeShader({ path }, defaultLaunchConfig)
{ }
ComputeShader::ComputeShader(std::initializer_list<const char *> paths, glm::uvec3 defaultLaunchConfig)
	: ShaderCore()
	, lastLaunchConfig(defaultLaunchConfig)
	, shaderFilePaths(paths)
{
	reload();
}
void ComputeShader::launch()
{
	if (lastLaunchConfig!= glm::uvec3(0)
		&& this->getProgram()>0)
	{
		//Setup shader
		this->useProgram();
		//Launch
		glDispatchCompute(lastLaunchConfig.x, lastLaunchConfig.y, lastLaunchConfig.z);
	}
}
glm::uvec3 ComputeShader::getMaxWorkGroupDims()
{
	int maxSize;
	glm::uvec3 rtn;
	GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSize));
	rtn.x = maxSize;
	GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSize));
	rtn.y = maxSize;
	GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSize));
	rtn.z = maxSize;
	return rtn;
}
unsigned int ComputeShader::getMaxThreadsPerWorkGroup()
{
	int maxInvoc;
	GL_CALL(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvoc));
	return maxInvoc;
}
bool ComputeShader::_compileShaders(const GLuint t_shaderProgram)
{
	return this->compileShader(t_shaderProgram, GL_COMPUTE_SHADER, shaderFilePaths);
}