#pragma once
class Shaders
{
public:
	Shaders(char* vertexShaderPath=0, char* fragmentShaderPath=0, char* geometryShaderPath=0);
	~Shaders();

	bool hasVertexShader();
	bool hasFragmentShader();
	bool hasGeometryShader();

	void createShaders();
	void useProgram();
	void clearProgram();

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

