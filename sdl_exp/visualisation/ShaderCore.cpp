#define  _CRT_SECURE_NO_WARNINGS
#include "ShaderCore.h"

ShaderCore::ShaderCore()
	: versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase)
	, compileSuccessFlag(true)
{
	
}

/*
Loads the text from the provided filepath
@return A pointer to the loaded shader source
@note the returned pointer is allocated via malloc, and should be free'd when nolonger required
*/
char* ShaderCore::loadShaderSource(const char* file){
	// If file path is 0 it is being omitted. kinda gross
	if (file != 0){
		FILE* fptr = fopen(file, "rb");
		if (!fptr){
			fprintf(stderr, "Shader not found: %s\n", file);
#ifdef EXIT_ON_ERROR
			getchar();
			exit(1);
#endif
			return 0;
		}
		fseek(fptr, 0, SEEK_END);
		long length = ftell(fptr);
		char* buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
		fseek(fptr, 0, SEEK_SET);
		fread(buf, length, 1, fptr);
		fclose(fptr);
		buf[length] = '\0'; // Null terminator
		return buf;
	}
	else {
		return 0;
	}
}
/*
Checks whether the specified shader compiled succesfully.
Compilation errors are printed to stderr and compileSuccessflag is set to false on failure.
@param shaderId Location of the shader to check
@param shaderPath Path to the shader being checked (so that it can be easily identified in the error log)
@return True if no errors were detected
*/
bool ShaderCore::checkShaderCompileError(int shaderId, const char* shaderPath){
	GL_CHECK();
	GLint status;
	GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status));
	if (status == GL_FALSE){
		// Get the length of the info log
		GLint len;
		GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len));
		// Get the contents of the log message
		char* log = new char[len + 1];
		GL_CALL(glGetShaderInfoLog(shaderId, len, &len, log));
		// Print the message
		fprintf(stderr, "Shader compilation error (%s) :\n", shaderPath);
		fprintf(stderr, "%s\n", log);
		delete[] log;
		this->compileSuccessFlag = false;
#ifdef EXIT_ON_ERROR
		getchar();
		exit(1);
#endif
		return false;
	}
	return true;
}
/*
Checks whether the specified shader program linked succesfully.
Linking errors are printed to stderr and compileSuccessflag is set to false on failure.
@param programId Location of the shader program to check
@return True if no errors were detected
@note For some reason program compilation failure logs don't seem to work (the same as shader compilation)
*/
bool ShaderCore::checkProgramCompileError(int programId){
	GL_CHECK();
	GLint status;
	GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &status));
	if (status == GL_FALSE){
		// Get the length of the info log
		GLint len = 0;
		GL_CALL(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &len));
		// Get the contents of the log message
		char* log = new char[len + 1];
		GL_CALL(glGetProgramInfoLog(programId, len, 0, log));
		// Print the message
		fprintf(stderr, "Program compilation error:\n");
		fprintf(stderr, "%s\n", log);
		delete[] log;
		this->compileSuccessFlag = false;
#if EXIT_ON_ERROR
		getchar();
		exit(1);
#endif
		return false;
	}
	return true;
}
/*
Looks for the '#version xx' tag in the provided shader source and returns the numeric value
@param shaderSource The shader code to detect the version from
@return The detected shader version, 0 if one was not found
*/
unsigned int ShaderCore::findShaderVersion(const char *shaderSource)
{
	std::cmatch match;
	//\#version ([0-9]+)\ <-versionRegex
	if (std::regex_search(shaderSource, match, this->versionRegex))
		return stoul(match[1]);
	return 0;
}
/*
Loads and compiles a shader
@param shaderPath The path to the shader to be loaded
@param shaderType The type of shader: GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
@return The shader program id, -1 if failure
@note This function will also update the value of compileSuccessFlag if failure occurs
 */
int ShaderCore::createShader(const char* shaderPath, GLenum shaderType)
{
	int rtn = glCreateShader(shaderType);
	char* shaderSrc = loadShaderSource(shaderPath);
	GL_CALL(glShaderSource(rtn, 1, &shaderSrc, 0));
	GL_CALL(glCompileShader(rtn));
	free(shaderSrc);
	if(this->checkShaderCompileError(rtn, shaderPath))
	{
		return rtn;
	}
	return -1;
}
