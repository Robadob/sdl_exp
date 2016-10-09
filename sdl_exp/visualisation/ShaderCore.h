#ifndef __ShaderCore_h__
#define __ShaderCore_h__

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "GLcheck.h"
#include <regex>

class ShaderCore
{
protected:
	ShaderCore();//This class can only be subclassed

	bool compileSuccessFlag;

	static char *loadShaderSource(const char *file);
	bool checkShaderCompileError(const int shaderId, const char *shaderPath);
	bool checkProgramCompileError(const int programId);
	std::regex versionRegex;
	unsigned int findShaderVersion(const char *shaderSource);
	int createShader(const char *shaderPath, GLenum shaderType);
};

#endif