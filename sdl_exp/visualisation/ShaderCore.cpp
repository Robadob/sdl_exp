#define  _CRT_SECURE_NO_WARNINGS
#include "ShaderCore.h"
#include <cstdlib> //<_splitpath() Windows only, need to rewrite linux ver
#include <regex>
#include <glm/gtc/type_ptr.hpp>

bool ShaderCore::exitOnError = false;//Tempted to use pre-processor macros to swap this default to true on release mode

//Constructors/Destructors
ShaderCore::ShaderCore()
	: programId(-1)
	, shaderTag("")
{ }
ShaderCore::~ShaderCore()
{
	if (shaderTag[0] != '\0') delete[] shaderTag;
}
//Core
void ShaderCore::reload()
{
	GL_CHECK();
	//Create temporary shader program
	GLuint t_programId = GL_CALL(glCreateProgram());
	//Pass it to subclass to compile shaders
	if (this->_compileShaders(t_programId))
	{
		// Link the program and ensure the program compiled correctly;
		GL_CALL(glLinkProgram(t_programId));

		// If the program linked ok, then we update the instance variable (for live reloading)
		if (this->checkProgramLinkError(t_programId)){
			// Destroy the old program
			this->destroyProgram();
			// Update the class var for the next usage.
			this->programId = t_programId;
		}
		else
		{
			//Compilation failed, cleanup temp program
			GL_CALL(glDeleteProgram(t_programId));
			deleteShaders();
			return;
		}
	}
	else
	{
		//Compilation failed, cleanup temp program
		GL_CALL(glDeleteProgram(t_programId));
		deleteShaders();
		return;
	}
	this->setupBindings();
}
void ShaderCore::setupBindings()
{
	//Refresh dynamic uniforms
	std::list<DynamicUniformDetail> t_dynamicUniforms;
	t_dynamicUniforms.splice(t_dynamicUniforms.end(), lostDynamicUniforms);
	for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); ++i)
	{
		t_dynamicUniforms.push_back(i->second);
	}
	dynamicUniforms.clear();
	for (DynamicUniformDetail d : t_dynamicUniforms)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, d.uniformName));
		if (location != -1)
		{
			dynamicUniforms.emplace(location,d);
		}
		else//If the buffer isn't found, remind the user
		{
			lostDynamicUniforms.push_front(d);
			printf("%s: Dynamic uniform '%s' could not be located on shader reload.\n", this->shaderTag, d.uniformName);
		}
	}
	//Refresh static uniforms
	glUseProgram(this->programId);
	for (std::list<StaticUniformDetail>::iterator i = staticUniforms.begin(); i != staticUniforms.end(); ++i)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, i->uniformName));
		if (location != -1)
		{
			if (i->type == GL_FLOAT)
			{
				if (sizeof(int) != sizeof(float))
					fprintf(stderr,"Error: int and float sizes differ, static float uniforms may be corrupted.\n");
				if (i->count == 1){
					GL_CALL(glUniform1fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
				}
				else if (i->count == 2){
					GL_CALL(glUniform2fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
				}
				else if (i->count == 3){
					GL_CALL(glUniform3fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
				}
				else if (i->count == 4){
					GL_CALL(glUniform4fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
				}
			}
			else if (i->type == GL_INT)
			{
				if (i->count == 1){
					GL_CALL(glUniform1iv(location, 1, glm::value_ptr(i->data)));
				}
				else if (i->count == 2){
					GL_CALL(glUniform2iv(location, 1, glm::value_ptr(i->data)));
				}
				else if (i->count == 3){
					GL_CALL(glUniform3iv(location, 1, glm::value_ptr(i->data)));
				}
				else if (i->count == 4){
					GL_CALL(glUniform4iv(location, 1, glm::value_ptr(i->data)));
				}
			}
		}
		else//If the uniform isn't found again, remind the user
		{
			printf("%s: Static uniform '%s' could not located on shader reload.\n", this->shaderTag, i->uniformName);
		}
	}
	//Refresh buffers
	std::list<BufferDetail> t_buffers;
	t_buffers.splice(t_buffers.end(), lostBuffers);
	for (std::map<GLuint, BufferDetail>::iterator i = buffers.begin(); i != buffers.end(); ++i)
	{
		t_buffers.push_back(i->second);
	}
	buffers.clear();
	for (BufferDetail d : t_buffers)
	{
		GLuint location = GL_CALL(glGetProgramResourceIndex(this->programId, d.type, d.nameInShader));
		if (location != GL_INVALID_INDEX)
		{			
			auto rtn = buffers.emplace(location, d);
			if (!rtn.second)fprintf(stderr,"Somehow a buffer was bound twice.");
		}
		else//If the buffer isn't found, remind the user
		{
			lostBuffers.push_front(d);
			printf("%s: Buffer '%s' could not be located on shader reload.\n", this->shaderTag, d.nameInShader);
		}
	}
	//Refresh subclass specific bindings
	this->_setupBindings();
	glUseProgram(0);
}
void ShaderCore::useProgram()
{
	//Kill if shader isn't built
	if (this->programId <= 0) reload();

	//glPushAttrib(GL_ALL_ATTRIB_BITS)? To shortern clearProgram?

	//Set any Texture buffers
	for (auto utd: textures)
	{
		glActiveTexture(GL_TEXTURE0 + utd.first);
		glBindTexture(utd.second.type, utd.second.name);
	}

	//Set any dynamic uniforms
	for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); ++i)
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
	//Set any buffers
	for (std::map<GLuint, BufferDetail>::iterator i = buffers.begin(); i != buffers.end(); ++i)
	{
		//Don't think buffer bases are specific to shaders, so we treat them as dynamic
		GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i->first, i->second.name));
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
	this->clearProgram();
	GL_CALL(glDeleteProgram(this->programId));
	this->programId = -1;
}
//Bindings
int ShaderCore::addTextureUniform(GLuint texture, char *uniformName, GLenum type)
{
	//Purge any existing buffer which matches
	for (auto a = textures.begin(); a != textures.end(); ++a)
	{
		if ((*a).second.name == texture)
		{
			a = textures.erase(a);
			--a;
		}
	}
	//Find the first free key	
	GLint bufferId = 0;
	for (bufferId; bufferId <= (GLint)textures.size();++bufferId)
	{
		//If bufferId doesn't exist, break
		if (textures.count(bufferId) == 0)
			break;
	}
	GLint maxTex;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTex);
	if (bufferId>=maxTex)
	{
		printf("Max texture buffers (%d) exceeded for shader %s.\n", maxTex, shaderTag);
	}
	if (addStaticUniform(uniformName, &bufferId))
	{
		UniformTextureDetail utd = { texture, type };
		textures.emplace(bufferId, utd);
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
	if (d.count > 0 && d.count <= 4)
	{
		//Purge any existing dynamic uniform which matches
		for (auto a = lostDynamicUniforms.begin(); a != lostDynamicUniforms.end(); ++a)
		{
			if (std::string((*a).uniformName) == std::string(d.uniformName))
			{
				a = lostDynamicUniforms.erase(a);
				--a;
			}
		}
		if (this->programId > 0)
		{
			GLint location = GL_CALL(glGetUniformLocation(this->programId, d.uniformName));
			if (location != -1)
			{
				//Replace with new one
				//Can't use[] assignment constructor due to const elements
				dynamicUniforms.erase(location);
				dynamicUniforms.emplace(location, d);
				return true;
			}
			fprintf(stderr, "%s: Dynamic uniform named: %s was not found.\n", shaderTag, d.uniformName);
		}
		lostDynamicUniforms.push_back(d);
	}
	return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count)
{
	//Purge any existing buffer which matches
	for (auto a = staticUniforms.begin(); a != staticUniforms.end(); ++a)
	{
		if (std::string((*a).uniformName) == std::string(uniformName))
		{
			a = staticUniforms.erase(a);
			--a;
		}
	}
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
			fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
		}
	}
	return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLint *arry, unsigned int count)
{
	//Purge any existing buffer which matches
	for (auto a = staticUniforms.begin(); a != staticUniforms.end(); ++a)
	{
		if (std::string((*a).uniformName) == std::string(uniformName))
		{
			a = staticUniforms.erase(a);
			--a;
		}
	}
	staticUniforms.push_front({ GL_INT, *reinterpret_cast<const glm::ivec4*>(arry), count, uniformName });
	if (this->programId > 0 && count > 0 && count <= 4)
	{
		GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
		if (location != -1)
		{
			glUseProgram(this->programId);
			if (count == 1){
				GL_CALL(glUniform1iv(location, 1, arry));
			}
			else if (count == 2){
				GL_CALL(glUniform2iv(location, 1, arry));
			}
			else if (count == 3){
				GL_CALL(glUniform3iv(location, 1, arry));
			}
			else if (count == 4){
				GL_CALL(glUniform4iv(location, 1, arry));
			}
			glUseProgram(0);
			return true;
		}
		else
		{
			fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
		}
	}
	return false;
}
bool ShaderCore::addBuffer(const char *bufferNameInShader, const GLenum bufferType, const GLuint bufferName)
{
	//Purge any existing buffer which matches
	for (auto a = lostBuffers.begin(); a != lostBuffers.end(); ++a)
	{
		if (std::string((*a).nameInShader) == std::string(bufferNameInShader))
		{
			a = lostBuffers.erase(a);
			--a;
		}
	}
	if (this->programId > 0)
	{
		GLuint blockIndex = GL_CALL(glGetProgramResourceIndex(this->programId, bufferType, bufferNameInShader));
		if (blockIndex != GL_INVALID_INDEX)
		{
			//Replace with new one
			//Can't use[] assignment constructor due to const elements
			BufferDetail bd = { bufferNameInShader, bufferType, bufferName };
			dynamicUniforms.erase(blockIndex);
			auto rtn = buffers.emplace(blockIndex, bd);
			if (!rtn.second)fprintf(stderr, "%s: Buffer named: %s is already bound.\n", shaderTag, bufferNameInShader);
			return true;
		}
		else
		{
			fprintf(stderr, "%s: Buffer named: %s was not found.\n", shaderTag, bufferNameInShader);
		}
	}
	lostBuffers.push_back({ bufferNameInShader, bufferType, bufferName });
	return false;
}
bool ShaderCore::removeDynamicUniform(const char *uniformName)
{
	bool rtn = false;
	for (auto a = lostDynamicUniforms.begin(); a != lostDynamicUniforms.end(); ++a)
	{
		if (std::string((*a).uniformName) == std::string(uniformName))
		{
			a = lostDynamicUniforms.erase(a);
			--a;
			rtn = true;
		}
	}
	for (auto a = dynamicUniforms.begin(); a != dynamicUniforms.end(); ++a)
	{
		if (std::string((*a).second.uniformName) == std::string(uniformName))
		{
			a = dynamicUniforms.erase(a);
			--a;
			rtn = true;
		}
	}
	return rtn;
}
bool ShaderCore::removeStaticUniform(const char *uniformName)
{
	bool rtn = false;
	for (auto a = staticUniforms.begin(); a != staticUniforms.end(); ++a)
	{
		if (std::string((*a).uniformName) == std::string(uniformName))
		{
			a = staticUniforms.erase(a);
			--a;
			rtn = true;
		}
	}
	return rtn;
}
bool ShaderCore::removeTextureUniform(const char *uniformName)
{
	bool rtn = false;
	for (auto a = staticUniforms.begin(); a != staticUniforms.end(); ++a)
	{
		if (std::string((*a).uniformName) == std::string(uniformName))
		{
			textures.erase((*a).data.x);
			a = staticUniforms.erase(a);
			--a;
			rtn = true;
		}
	}
	return rtn;
}
bool ShaderCore::removeBuffer(const char *nameInShader)
{
	bool rtn = false;
	for (auto a = lostBuffers.begin(); a != lostBuffers.end(); ++a)
	{
		if (std::string((*a).nameInShader) == std::string(nameInShader))
		{
			a = lostBuffers.erase(a);
			--a;
			rtn = true;
		}
	}
	for (auto a = buffers.begin(); a != buffers.end(); ++a)
	{
		if (std::string((*a).second.nameInShader) == std::string(nameInShader))
		{
			a = buffers.erase(a);
			--a;
			rtn = true;
		}
	}
	return rtn;
}
std::pair<int, GLenum> ShaderCore::findUniform(const char *uniformName, const int shaderProgram)
{
	int result = shaderProgram<0 ? -1 : GL_CALL(glGetUniformLocation(shaderProgram, uniformName));
	if (result > -1)
	{
		GLenum type;
		GLint size;//Collect size, because its not documented that you can pass 0
		GL_CALL(glGetActiveUniform(shaderProgram, result, 0, nullptr, &size, &type, nullptr));
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
		GL_CALL(glGetActiveAttrib(shaderProgram, result, 0, nullptr, &size, &type, nullptr));
		return std::pair<int, GLenum>(result, type);
	}
	return  std::pair<int, GLenum>(-1, 0);
}
//Util
bool ShaderCore::compileShader(const GLuint t_shaderProgram, GLenum type, std::initializer_list<const char *> shaderSourceFiles)
{
	if (shaderSourceFiles.size() == 0) return false;
	// Load shader files
	std::vector<char*> shaderSources;
	for (auto i : shaderSourceFiles)
	{
		shaderSources.push_back(loadShaderSource(i));
	}
	//Check for shaders that didn't load correctly
	for (auto i : shaderSources)
	{
		if (!i)
		{
			//Cleanup
			for (auto j : shaderSources)
			{
				free(j);
			}
			return false;
		}
	}
	GLuint shaderId = createShader(type);
	GL_CALL(glShaderSource(shaderId, shaderSources.size(), &shaderSources[0], nullptr));
	GL_CALL(glCompileShader(shaderId));
	std::string shaderName = getFilenameFromPath(*(shaderSourceFiles.end() - 1));
	//Check for compile errors
	if (!this->checkShaderCompileError(shaderId, shaderName.c_str()))
	{
		//Cleanup
		for (auto j : shaderSources)
		{
			free(j);
		}
		return false;
	}
	//Attach shader to program
	GL_CALL(glAttachShader(t_shaderProgram, shaderId));
	//Append to shaderTag
	if (shaderTag[0] == '\0')
	{
		shaderName = removeFileExt(shaderName);
	}
	else
	{
		shaderName = std::string(shaderTag) + std::string(":") + removeFileExt(shaderName);
		delete[] this->shaderTag;
	}
	this->shaderTag = new char[shaderName.length() + 1];
	strcpy(this->shaderTag, shaderName.c_str());
	return true;
}
char* ShaderCore::loadShaderSource(const char* file){
	// If file path is 0 it is being omitted. kinda gross
	if (file != nullptr){
		FILE* fptr = fopen(file, "rb");
		if (!fptr){
			fprintf(stderr, "Shader source not found: %s\n", file);
			if(exitOnError)
			{
				getchar();
				exit(1);
			}
			return nullptr;
		}
		fseek(fptr, 0, SEEK_END);
		long length = ftell(fptr);
		char* buf = static_cast<char*>(malloc(length + 1)); // Allocate a buffer for the entire length of the file and a null terminator
		fseek(fptr, 0, SEEK_SET);
		fread(buf, length, 1, fptr);
		fclose(fptr);
		buf[length] = '\0'; // Null terminator
		return buf;
	}
	else {
		return nullptr;
	}
}
bool ShaderCore::checkShaderCompileError(GLuint shaderId, const char* shaderPath){
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
		if(exitOnError)
		{
			getchar();
			exit(1);
		}
		return false;
	}
	return true;
}
bool ShaderCore::checkProgramLinkError(const GLuint programId) const{
	GL_CHECK();
	GLint status;
	GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &status));
	if (status == GL_FALSE){
		// Get the length of the info log
		GLint len = 0;
		GL_CALL(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &len));
		// Get the contents of the log message
		char* log = new char[len + 1];
		GL_CALL(glGetProgramInfoLog(programId, len, nullptr, log));
		// Print the message
		fprintf(stderr, "Program compilation error (%s):\n", shaderTag);
		fprintf(stderr, "%s\n", log);
		delete[] log;
		if(exitOnError)
		{
			getchar();
			exit(1);
		}
		return false;
	}
	return true;
}
//unsigned int ShaderCore::findShaderVersion(const char *shaderSource)
//{
//	static std::regex versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase);
//	std::cmatch match;
//	if (std::regex_search(shaderSource, match, versionRegex))
//		return stoul(match[1]);
//	return 0;
//}
//_WIN32 is defined for both x86 and x64
//https://msdn.microsoft.com/en-us/library/b0084kay.aspx
#ifdef _WIN32 
struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '\\' || ch == '/';
	}
};
#else
struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '/';
	}
};
#endif
std::string ShaderCore::getFilenameFromPath(const char* filePath)
{
	std::string pathname(filePath);
	std::string result = std::string(
		std::find_if(pathname.rbegin(), pathname.rend(),
		MatchPathSeparator()).base(),
		pathname.end());
	printf("File: %s\n", filePath);
	printf("FileName: %s\n", result.c_str());
	return result;
}
std::string ShaderCore::removeFileExt(const std::string &filename)
{
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos) return filename;
	printf("File: %s\n", filename.c_str());
	printf("FileName: %s\n", filename.substr(0, lastdot).c_str());
	return filename.substr(0, lastdot);
}