#include "ComputeShader.h"

ComputeShader::ComputeShader(const char *path, glm::uvec3 defaultLaunchConfig)
	: ComputeShader(std::initializer_list<std::string>{ path }, defaultLaunchConfig)
{ }
ComputeShader::ComputeShader(std::initializer_list<std::string> paths, glm::uvec3 defaultLaunchConfig)
	: ShaderCore()
	, lastLaunchConfig(defaultLaunchConfig)
	, shaderFilePaths(buildFileVector(paths))
{
	reload();
}
ComputeShader::ComputeShader(const ComputeShader&other)
	: ShaderCore(other)
	, lastLaunchConfig(other.lastLaunchConfig)
	, shaderFilePaths(nullptr)
{
	shaderFilePaths = new std::vector<std::string>();
	for (const auto &i : *other.shaderFilePaths)
		shaderFilePaths->push_back(std::string(i));
	reload();
}
ComputeShader::~ComputeShader()
{
	this->destroyProgram();
	delete shaderFilePaths;
}
void ComputeShader::launch()
{
	if (lastLaunchConfig!= glm::uvec3(0)
		&& this->getProgram()>0)
	{
		//Setup shader
		this->useProgram();
		//Launch
        GL_CALL(glDispatchCompute(lastLaunchConfig.x, lastLaunchConfig.y, lastLaunchConfig.z));
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
	return this->compileShader(t_shaderProgram, GL_COMPUTE_SHADER, shaderFilePaths)>=0;
}