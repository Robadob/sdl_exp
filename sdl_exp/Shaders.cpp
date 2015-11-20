#define  _CRT_SECURE_NO_WARNINGS
#include "Shaders.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <GL/glew.h>

#define EXIT_ON_ERROR 0



Shaders::Shaders(char* vertexShaderPath, char* fragmentShaderPath, char* geometryShaderPath) : compileSuccessFlag(true){
	// This is a bit pointless but why not.
	this->vertexShaderPath = vertexShaderPath;
	this->fragmentShaderPath = fragmentShaderPath;
	this->geometryShaderPath = geometryShaderPath;
	// Create the shaders
	this->createShaders();
	//this->useProgram();
	this->checkGLError();

}

Shaders::~Shaders(){
	this->destroyProgram();
}

bool Shaders::hasVertexShader(){
	return this->vertexShaderPath!=0;
}

bool Shaders::hasFragmentShader(){
    return this->fragmentShaderPath != 0;
}

bool Shaders::hasGeometryShader(){
    return this->geometryShaderPath != 0;
}


void Shaders::createShaders(){
	// Reset the flag
	this->compileSuccessFlag = true;
	// Load shader files
	const char* vertexSource = loadShaderSource(this->vertexShaderPath);
	const char* fragmentSource = loadShaderSource(this->fragmentShaderPath);
	const char* geometrySource = loadShaderSource(this->geometryShaderPath);
	//Check for shaders that didn't load correctly
	if (vertexSource == 0) this->vertexShaderPath = 0;
	if (fragmentSource == 0) this->fragmentShaderPath = 0;
	if (geometrySource == 0) this->geometryShaderPath = 0;

	// For each shader we have been able to read.
	// Create the empty shader handle
	// Atrtempt to compile the shader
	// Check compilation
	// If it fails, bail out.
	if (hasVertexShader()){
		this->vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		//printf("\n>>vsi: %d\n", this->vertexShaderId);
		glShaderSource(this->vertexShaderId, 1, &vertexSource, 0);
		this->checkGLError();
		glCompileShader(this->vertexShaderId);
		this->checkGLError();
		this->checkShaderCompileError(this->vertexShaderId, this->vertexShaderPath);

	}
	if (hasFragmentShader()){
		this->fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(this->fragmentShaderId, 1, &fragmentSource, 0);
		this->checkGLError();
		glCompileShader(this->fragmentShaderId);
		this->checkGLError();
		this->checkShaderCompileError(this->fragmentShaderId, this->fragmentShaderPath);
	}
	if (hasGeometryShader()){
		this->geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(this->geometryShaderId, 1, &geometrySource, 0);
		this->checkGLError();
		glCompileShader(this->geometryShaderId);
		this->checkGLError();
		this->checkShaderCompileError(this->geometryShaderId, this->geometryShaderPath);
	}

	// Only attempt to link the program if the compilation of each individual shader was successful.
	if (this->compileSuccessFlag){

		// Create the program
		int newProgramId = glCreateProgram();
		this->checkGLError();

		// Attach each included shader
		if (this->hasVertexShader()){
			glAttachShader(newProgramId, this->vertexShaderId);
			this->checkGLError();
		}
		if (this->hasFragmentShader()){
			glAttachShader(newProgramId, this->fragmentShaderId);
			this->checkGLError();
		}
		if (this->hasGeometryShader()){
			glAttachShader(newProgramId, this->geometryShaderId);
			this->checkGLError();
		}
		// Link the program and Ensure the program compiled correctly;
		glLinkProgram(newProgramId);
		this->checkGLError();

		this->checkProgramCompileError(newProgramId);
		this->checkGLError();
		// If the program compiled ok, then we update the instance variable (for live reloading
		if (this->compileSuccessFlag){
			// Destroy the old program
			this->destroyProgram();
			this->checkGLError();
			// Update the class var for the next usage.
			this->programId = newProgramId;
			this->checkGLError();
		}
	}
	this->checkGLError();

	// Clean up any shaders
	this->destroyShaders();
	// and delete sources
	delete vertexSource;
	delete fragmentSource;
	delete geometrySource;
}

void Shaders::reloadShaders(){
	fprintf(stdout, "Reloading Shaders\n");
	this->createShaders();
}

void Shaders::useProgram(){
	glUseProgram(this->programId);

	glBindAttribLocation(this->programId, 0, "in_position");
	this->checkGLError();

	////glBindAttribLocation(this->programId, 1, "in_normal");
	//this->checkGLError();

	//GLfloat model[16];
	//glGetFloatv(GL_MODELVIEW_MATRIX, model);
	//this->setUniformMatrix4fv(1, model);
	////glUniformMatrix4fv(1, 1, GL_FALSE, model);
	//this->checkGLError();
	//glGetFloatv(GL_PROJECTION_MATRIX, model);
	//this->setUniformMatrix4fv(2, model);
	////glUniformMatrix4fv(2, 1, GL_FALSE, model);
	//this->checkGLError(); 
}

void Shaders::clearProgram(){
	glUseProgram(0);
}

void Shaders::setUniformi(int location, int value){
	if (location >= 0){
		glUniform1i(location, value);
	}
}

void Shaders::setUniformMatrix4fv(int location, GLfloat* value){
	if (location >= 0){
		// Must be false and length with most likely just be 1. Can add an extra parameter version if required.
		glUniformMatrix4fv(location, 1, GL_FALSE, value);
	}
}

char* Shaders::loadShaderSource(char* file){
	// If file path is 0 it is being omitted. kinda gross
	if (file != 0){
		FILE* fptr = fopen(file, "rb");
		if (!fptr){
			printf("Shader not found: %s\n", file);
#if EXIT_ON_ERROR == 1
			//@todo exit maybe?
			system("pause"); // @temp for pausing on output.
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
		buf[length] = 0; // Null terminator
		return buf;
	}
	else {
		return 0;
	}
}


void Shaders::destroyShaders(){
	// Destroy the shaders and program
	if (this->hasVertexShader()){
		glDeleteShader(this->vertexShaderId);
		this->checkGLError();
	}
	if (this->hasFragmentShader()){
		glDeleteShader(this->fragmentShaderId);
		this->checkGLError();
	}
	if (this->hasGeometryShader()){
		glDeleteShader(this->geometryShaderId);
		this->checkGLError();
	}
}

void Shaders::destroyProgram(){
	glDeleteProgram(this->programId);
}

void Shaders::checkGLError(){
	GLuint error = glGetError();
	if (error != GL_NO_ERROR)
	{
		const char* errMessage = (const char*)gluErrorString(error);
		fprintf(stderr, "(shaders)OpenGL Error #%d: %s\n", error, errMessage);
	}
}

void Shaders::checkShaderCompileError(int shaderId, char* shaderPath){
	this->checkGLError();

	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE){
		// Get the length of the info log
		GLint len;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len);
		// Get the contents of the log message
		char* log = new char[len + 1];
		glGetShaderInfoLog(shaderId, len, &len, log);
		// Print the message
		printf("Shader compilation error (%s) :\n", shaderPath);
		printf("%s\n", log);
		delete log;
		this->compileSuccessFlag = false;
#if EXIT_ON_ERROR == 1
		//@todo exit maybe?
		system("pause"); // @temp for pausing on output.
		exit(1);
#endif
	}
	
}



void Shaders::checkProgramCompileError(int programId){
	int status;
	this->checkGLError();
	glGetProgramiv(programId, GL_LINK_STATUS, &status);
	this->checkGLError();
	if (status == GL_FALSE){
		// Get the length of the info log
		GLint len = 0;
		glGetProgramiv(this->programId, GL_INFO_LOG_LENGTH, &len);
		// Get the contents of the log message
		char* log = new char[len + 1];
		glGetProgramInfoLog(this->programId, len, NULL, log);
		// Print the message
		printf("Program compilation error:\n");
		printf("%s\n", log);
		this->compileSuccessFlag = false;
		this->checkGLError();
#if EXIT_ON_ERROR == 1
		//@todo exit maybe?
		system("pause"); // @temp for pausing on output.
		exit(1);
#endif
	}
}
