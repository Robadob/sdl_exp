#define  _CRT_SECURE_NO_WARNINGS
#include "ShaderCore.h"
#include <regex>

ShaderCore::ShaderCore(const char *shaderTag)
	: compileSuccessFlag(false)
	, programId(-1)
	, exitOnError(false)//Tempted to use pre-processor macros to swap this default to true on release mode
	, shaderTag(shaderTag)
{ }
ShaderCore::~ShaderCore()
{
	TODO
}
void ShaderCore::reload()
{
	TODO init
	this->_reload();
	TODO more init
}
//Todo, build shader if not ready?
void ShaderCore::useProgram()
{
	//Kill if shader isn't built
	if (this->programId < 0) return;

	//glPushAttrib(GL_ALL_ATTRIB_BITS)? To shortern clearProgram?

	//Set any Texture buffers
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + textures[i].bufferId);
		glBindTexture(textures[i].type, textures[i].name);
	}

	//Set any dynamic uniforms
	for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); i++)
	{
		if (i->second.type == GL_FLOAT)
		{
			if (i->second.count == 1)
			{
				GL_CALL(glUniform1fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
			}
			else if (i->second.count == 2){
				GL_CALL(glUniform2fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
			}
			else if (i->second.count == 3){
				GL_CALL(glUniform3fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
			}
			else if (i->second.count == 4){
				GL_CALL(glUniform4fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
			}
		}
		else if (i->second.type == GL_INT)
		{
			if (i->second.count == 1){
				GL_CALL(glUniform1iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
			}
			else if (i->second.count == 2){
				GL_CALL(glUniform2iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
			}
			else if (i->second.count == 3){
				GL_CALL(glUniform3iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
			}
			else if (i->second.count == 4){
				GL_CALL(glUniform4iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
			}
		}
	}

	//Set any subclass specific stuff
	this->_useProgram();
}
void ShaderCore::clearProgram()
{
	this->_clearProgram();
	GL_CALL(glUseProgram(0));
}
void ShaderCore::destroyProgram()
{
	GL_CALL(glDeleteProgram(this->programId));
	this->programId = -1;
	compileSuccessFlag = false;
}
int ShaderCore::addTextureUniform(GLuint texture, char *uniformName, GLenum type)
{
	GLint bufferId = (GLint)textures.size();
	GLint maxTex;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTex);
	if (bufferId<maxTex && addStaticUniform(uniformName, &bufferId))
	{
		textures.push_back({ texture, bufferId, type });
		return bufferId;
	}
	return -1;
}
bool ShaderCore::addDynamicUniform(const char *uniformName, const GLint *arry, unsigned int count)
{
	return addDynamicUniform({ GL_INT, reinterpret_cast<const void*>(arry), count, uniformName });
}
bool ShaderCore::addDynamicUniform(const char *uniformName, const GLfloat *arry, unsigned int count)
{
	return addDynamicUniform({ GL_FLOAT, reinterpret_cast<const void*>(arry), count, uniformName });
}
bool ShaderCore::addDynamicUniform(DynamicUniformDetail d)
{
	if (this->programId > 0 && d.count > 0 && d.count <= 4)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, d.uniformName));
		if (location != -1)
		{
			dynamicUniforms[location] = d;
			return true;
		}
		lostDynamicUniforms.push_back(d);
		fprintf(stderr, "Dynamic uniform named: %s was not found in shaders '%s' etc.\n", d.uniformName, shaderTag);
	}
	return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count)
{
	//Note we reinterpret_cast the data to from float to int
	staticUniforms.push_front({ GL_FLOAT, *reinterpret_cast<const glm::ivec4*>(arry), count, uniformName });
	if (this->programId > 0 && count > 0 && count <= 4)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
		if (location != -1)
		{
			glUseProgram(this->programId);
			if (count == 1){
				GL_CALL(glUniform1fv(location, 1, arry));
			}
			else if (count == 2){
				GL_CALL(glUniform2fv(location, 1, arry));
			}
			else if (count == 3){
				GL_CALL(glUniform3fv(location, 1, arry));
			}
			else if (count == 4){
				GL_CALL(glUniform4fv(location, 1, arry));
			}
			glUseProgram(0);
			return true;
		}
		else
		{
			fprintf(stderr, "Static uniform named: %s was not found in shaders '%s' etc.\n", uniformName, shaderTag);
		}
	}
	return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLint *array, unsigned int count)
{
	staticUniforms.push_front({ GL_INT, *reinterpret_cast<const glm::ivec4*>(array), count, uniformName });
	if (this->programId > 0 && count > 0 && count <= 4)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
		if (location != -1)
		{
			glUseProgram(this->programId);
			if (count == 1){
				GL_CALL(glUniform1iv(location, 1, array));
			}
			else if (count == 2){
				GL_CALL(glUniform2iv(location, 1, array));
			}
			else if (count == 3){
				GL_CALL(glUniform3iv(location, 1, array));
			}
			else if (count == 4){
				GL_CALL(glUniform4iv(location, 1, array));
			}
			glUseProgram(0);
			return true;
		}
		else
		{
			fprintf(stderr, "Static uniform named: %s was not found in shaders '%s' etc.\n", uniformName, shaderTag);
		}
	}
	return false;
}

std::pair<int, GLenum> ShaderCore::findUniform(const char *uniformName, const int shaderProgram)
{
	int result = shaderProgram<0 ? -1 : GL_CALL(glGetUniformLocation(shaderProgram, uniformName));
	if (result > -1)
	{
		GLenum type;
		GLint size;//Collect size, because its not documented that you can pass 0
		GL_CALL(glGetActiveUniform(shaderProgram, result, 0, 0, &size, &type, 0));
		return std::pair<int, GLenum>(result, type);
	}
	return  std::pair<int, GLenum>(-1, 0);
}
std::pair<int, GLenum> ShaderCore::findAttribute(const char *attributeName, const int shaderProgram)
{
	int result = shaderProgram<0 ? -1 : GL_CALL(glGetAttribLocation(shaderProgram, attributeName));
	if (result > -1)
	{
		GLenum type;
		GLint size;//Collect size, because its not documented that you can pass 0
		GL_CALL(glGetActiveAttrib(shaderProgram, result, 0, 0, &size, &type, 0));
		return std::pair<int, GLenum>(result, type);
	}
	return  std::pair<int, GLenum>(-1, 0);
}
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
unsigned int ShaderCore::findShaderVersion(const char *shaderSource)
{
	static std::regex versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase);
	std::cmatch match;
	if (std::regex_search(shaderSource, match, versionRegex))
		return stoul(match[1]);
	return 0;
}

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
