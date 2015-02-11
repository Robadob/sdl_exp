#pragma once

#include <GL/glew.h>

class Shaders
{
public:
	Shaders(char* vertexShaderPath=0, char* fragmentShaderPath=0, char* geometryShaderPath=0);
	~Shaders();

	bool hasVertexShader();
	bool hasFragmentShader();
	bool hasGeometryShader();

	void createShaders();
	void reloadShaders();
	void useProgram();
	void clearProgram();
	void setUniformi(int location, int value);
	void setUniformMatrix4fv(int location, GLfloat* value);

private:
	char* vertexShaderPath;
	char* fragmentShaderPath;
	char* geometryShaderPath;

	int vertexShaderId;
	int fragmentShaderId;
	int geometryShaderId;
	int programId;

	char* loadShaderSource(char* file);
	void destroyShaders();
	void destroyProgram();
	void checkGLError();
	void checkShaderCompileError(int shaderId, char* shaderPath);
	void checkProgramCompileError();

};

