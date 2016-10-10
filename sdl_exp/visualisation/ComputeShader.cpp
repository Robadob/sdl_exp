#include "ComputeShader.h"

ComputeShader::ComputeShader()
	: programId(0)
	, lastLaunchConfig(0)
{
	reload();
}

int ComputeShader::addShader(const char* shaderPath)
{
	shaders.push_back({shaderPath});
	return reload() ? shaders.size() : -1;
}

bool ComputeShader::reload()
{
	GL_CHECK(); 
	// Reset the flag
	this->compileSuccessFlag = true;
	//Return if no shaders
	if (shaders.empty())
		return true;
	//Load each shader
	for (int i = 0; i < shaders.size(); ++i)
	{
		shaders[i].shaderId = createShader(shaders[i].shaderPath, GL_COMPUTE_SHADER);
	}
	// Only attempt to link the program if the compilation of each individual shader was successful.
	if (this->compileSuccessFlag){

		// Create the program
		int newProgramId = GL_CALL(glCreateProgram());

		// Attach each included shader
		for (int i = 0; i < shaders.size();++i)
		{
			GL_CALL(glAttachShader(newProgramId, shaders[i].shaderId));
		}
		
		// Link the program and Ensure the program compiled correctly;
		GL_CALL(glLinkProgram(newProgramId));

		this->checkProgramCompileError(newProgramId);
		// If the program compiled ok, then we update the instance variable (for live reloading)
		if (this->compileSuccessFlag){
			// Destroy the old program
			GL_CALL(glDeleteProgram(this->programId));
			// Update the class var for the next usage.
			this->programId = newProgramId;
		}
	}
	return this->compileSuccessFlag;
}
void ComputeShader::launch()
{
	if (lastLaunchConfig!= glm::uvec3(0)
		&& this->compileSuccessFlag
			&& shaders.size())
	{
		GL_CALL(glUseProgram(this->programId)); 
		//Bind any uniforms

		//Launch
		glDispatchCompute(lastLaunchConfig.x, lastLaunchConfig.y, lastLaunchConfig.z);
	}
}
/*
 Returns the maximum launch dimensions in each axis
 @return Probably (1536,1024,64)
 @see GL_MAX_COMPUTE_WORK_GROUP_SIZE
*/
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
/*
  Returns the maximum number of threads in each work group
  @return Probably 1536
  @see GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS
*/
unsigned int ComputeShader::getMaxThreadsPerWorkGroup()
{
	int maxInvoc;
	GL_CALL(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvoc));
	return maxInvoc;
}