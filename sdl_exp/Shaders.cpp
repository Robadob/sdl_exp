#define  _CRT_SECURE_NO_WARNINGS

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "Shaders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>

/*
Constructs a shader program from the provided shader files
@param vertexShaderPath Path to the GLSL vertex shader (nullptr if not required)
@param fragmentShaderPath Path to the GLSL fragment shader (nullptr if not required)
@param geometryShaderPath Path to the GLSL geometry shader (nullptr if not required)
*/
Shaders::Shaders(char* vertexShaderPath, char* fragmentShaderPath, char* geometryShaderPath)
    : compileSuccessFlag(true)
    , vertexShaderPath(vertexShaderPath)
    , fragmentShaderPath(fragmentShaderPath)
    , geometryShaderPath(geometryShaderPath)
{
    this->createShaders();
    GL_CHECK();
}
/*
Destructor, free's the shader program
*/
Shaders::~Shaders(){
    this->destroyProgram();
}
/*
Returns whether a vertex shader has been loaded
@return True if a vertex shader is present, else false
*/
bool Shaders::hasVertexShader() const
{
    return this->vertexShaderPath!=0;
}
/*
Returns whether a fragment shader has been loaded
@return True if a fragment shader is present, else false
*/
bool Shaders::hasFragmentShader() const
{
    return this->fragmentShaderPath != 0;
}
/*
Returns whether a geometry shader has been loaded
@return True if a geometry shader is present, else false
*/
bool Shaders::hasGeometryShader() const{
    return this->geometryShaderPath != 0;
}
/*
Returns whether the last call to createShaders() (via object creation or a call to reloadShaders()) was successful
@return True if the currently loaded shaders match the last loaded shaders
*/
bool Shaders::getCompileSuccess() const{
    return this->compileSuccessFlag;
}
/*
Compiles the shader sources specified at the objects creation
Errors are reported directly to stderr, and will cause compileSuccessFlag to be set to false
Exiting the program on error can be configured by defining EXIT_ON_ERROR
*/
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
    // Attempt to compile the shader
    // Check compilation
    if (hasVertexShader()){
        this->vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        //printf("\n>>vsi: %d\n", this->vertexShaderId);
        GL_CALL(glShaderSource(this->vertexShaderId, 1, &vertexSource, 0));
        GL_CALL(glCompileShader(this->vertexShaderId));
        this->checkShaderCompileError(this->vertexShaderId, this->vertexShaderPath);

    }
    if (hasFragmentShader()){
        this->fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CALL(glShaderSource(this->fragmentShaderId, 1, &fragmentSource, 0));
        GL_CALL(glCompileShader(this->fragmentShaderId));
        this->checkShaderCompileError(this->fragmentShaderId, this->fragmentShaderPath);
    }
    if (hasGeometryShader()){
        this->geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
        GL_CALL(glShaderSource(this->geometryShaderId, 1, &geometrySource, 0));
        GL_CALL(glCompileShader(this->geometryShaderId));
        this->checkShaderCompileError(this->geometryShaderId, this->geometryShaderPath);
    }

    // Only attempt to link the program if the compilation of each individual shader was successful.
    if (this->compileSuccessFlag){

        // Create the program
        int newProgramId = GL_CALL(glCreateProgram());

        // Attach each included shader
        if (this->hasVertexShader())
            GL_CALL(glAttachShader(newProgramId, this->vertexShaderId));
        
        if (this->hasFragmentShader())
            GL_CALL(glAttachShader(newProgramId, this->fragmentShaderId));
        
        if (this->hasGeometryShader())
            GL_CALL(glAttachShader(newProgramId, this->geometryShaderId));
        
        // Link the program and Ensure the program compiled correctly;
        GL_CALL(glLinkProgram(newProgramId));

        this->checkProgramCompileError(newProgramId);
        // If the program compiled ok, then we update the instance variable (for live reloading)
        if (this->compileSuccessFlag){
            // Destroy the old program
            this->destroyProgram();
            // Update the class var for the next usage.
            this->programId = newProgramId;
        }
    }

    // Clean up any shaders
    this->destroyShaders();
    // and delete sources
    delete vertexSource;
    delete fragmentSource;
    delete geometrySource;
}
/*
Reloads the shaders live, according to the file paths provided at the objects creation
If shader creation fails, the existing shaders will continue to be used
@param silent When false 'Reloading shaders' will be printed to console
@return True when the new shaders were loaded successfully
*/
bool Shaders::reloadShaders(bool silent){
    if (!silent)
        printf("Reloading Shaders\n");
    this->createShaders();
    return this->compileSuccessFlag;
}
/*
Call this prior to rendering to enable the program and automatically bind known attributes
*/
void Shaders::useProgram(){
    GL_CALL(glUseProgram(this->programId));

    GL_CALL(glBindAttribLocation(this->programId, 0, "in_position"));

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
/*
Calls glUseProgram(0) to disable the currently active shader progam
*/
void Shaders::clearProgram(){
    GL_CALL(glUseProgram(0));
}
/*
Sets a uniform integer value
@param location The uniform location
@param value The value to be set
@see glUniform1i()
*/
void Shaders::setUniformi(int location, int value){
    if (location >= 0){
        glUniform1i(location, value);
    }
}
/*
Sets a uniform matrix value
@param location The uniform location
@param value The value to be set
@see glUniformMatrix4fv()
*/
void Shaders::setUniformMatrix4fv(int location, GLfloat* value){
    if (location >= 0){
        // Must be false and length with most likely just be 1. Can add an extra parameter version if required.
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}
/*
Loads the text from the provided filepath
@return A pointer to the loaded shader source
@note the returned pointer is allocated via malloc, and should be free'd when nolonger required
*/
char* Shaders::loadShaderSource(char* file){
    // If file path is 0 it is being omitted. kinda gross
    if (file != 0){
        FILE* fptr = fopen(file, "rb");
        if (!fptr){
            fprintf(stderr,"Shader not found: %s\n", file);
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
Destroys the compiled shaders
This is called after the shader program has been linked (not in the destructor), they are nolonger required after that point
*/
void Shaders::destroyShaders(){
    if (this->hasVertexShader())
        GL_CALL(glDeleteShader(this->vertexShaderId));

    if (this->hasFragmentShader())
        GL_CALL(glDeleteShader(this->fragmentShaderId));

    if (this->hasGeometryShader())
        GL_CALL(glDeleteShader(this->geometryShaderId));
}
/*
Destroys the shader program
*/
void Shaders::destroyProgram(){
    GL_CALL(glDeleteProgram(this->programId));
}
/*
Checks whether the specified shader compiled succesfully.
Compilation errors are printed to stderr and compileSuccessflag is set to false on failure.
@param shaderId Location of the shader to check
@param shaderPath Path to the shader being checked (so that it can be easily identified in the error log)
@return True if no errors were detected
*/
bool Shaders::checkShaderCompileError(int shaderId, char* shaderPath){
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
        delete log;
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
bool Shaders::checkProgramCompileError(int programId){
    GL_CHECK();
    GLint status;
    GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &status));
    if (status == GL_FALSE){
        // Get the length of the info log
        GLint len = 0;
        GL_CALL(glGetProgramiv(this->programId, GL_INFO_LOG_LENGTH, &len));
        // Get the contents of the log message
        char* log = new char[len + 1];
        GL_CALL(glGetProgramInfoLog(this->programId, len, NULL, log));
        // Print the message
        fprintf(stderr, "Program compilation error:\n");
        fprintf(stderr, "%s\n", log);
        this->compileSuccessFlag = false;
#if EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
        return false;
    }
    return true;
}
