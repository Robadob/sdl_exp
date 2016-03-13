#define  _CRT_SECURE_NO_WARNINGS

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "Shaders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <glm\gtc\type_ptr.hpp>

/*
Constructs a shader program from the provided shader files
@param vertexShaderPath Path to the GLSL vertex shader (nullptr if not required)
@param fragmentShaderPath Path to the GLSL fragment shader (nullptr if not required)
@param geometryShaderPath Path to the GLSL geometry shader (nullptr if not required)
@param camera The camera object to pull the modelView and projection matrices from
*/
Shaders::Shaders(char *vertexShaderPath, char *fragmentShaderPath, char *geometryShaderPath)
    : compileSuccessFlag(true)
    , vertexShaderPath(vertexShaderPath)
    , fragmentShaderPath(fragmentShaderPath)
    , geometryShaderPath(geometryShaderPath)
    , vertexShaderVersion(0)
    , fragmentShaderVersion(0)
    , geometryShaderVersion(0)
    , versionRegex("#version ([0-9]+)", std::regex_constants::icase)
    , projectionMatrixUniformLocation(-1)
    , modelviewMatrixUniformLocation(-1)
    , vertexAttributeLocation(-1)
    , normalAttributeLocation(-1)
    , colorAttributeLocation(-1)
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

    //Compilation was successful, lets try and detect how to configure the shader
    if (this->compileSuccessFlag)
    {
        //Detect shader version numbers
        if (this->hasVertexShader())
            this->vertexShaderVersion = this->findShaderVersion(vertexSource);
        if (this->hasFragmentShader())
            this->fragmentShaderVersion = this->findShaderVersion(fragmentSource);
        if (this->hasGeometryShader())
            this->geometryShaderVersion = this->findShaderVersion(geometrySource);

        //Locate the modelView matrix uniform
        std::pair<int, GLenum> u_MV = findUniform(MODELVIEW_MATRIX_UNIFORM_NAME, this->programId);
        if (u_MV.first >= 0 && u_MV.second == GL_FLOAT_MAT4)
            modelviewMatrixUniformLocation = u_MV.first;
        else
            modelviewMatrixUniformLocation = -1;
        //Locate the projection matrix uniform
        std::pair<int, GLenum> u_PM = findUniform(PROJECTION_MATRIX_UNIFORM_NAME, this->programId);
        if (u_PM.first >= 0 && u_PM.second == GL_FLOAT_MAT4)
            projectionMatrixUniformLocation = u_PM.first;
        else
            projectionMatrixUniformLocation = -1;
        //Locate the vertexPosition attribute
        std::pair<int, GLenum> a_V = findAttribute(VERTEX_ATTRIBUTE_NAME, this->programId);
        if (a_V.first >= 0 && (a_V.second == GL_FLOAT_VEC3 || a_V.second == GL_FLOAT_VEC4))
            vertexAttributeLocation = a_V.first;
        else
            vertexAttributeLocation = -1;
        //Locate the vertexNormal attribute
        std::pair<int, GLenum> a_N = findAttribute(NORMAL_ATTRIBUTE_NAME, this->programId);
        if (a_N.first >= 0 && (a_N.second == GL_FLOAT_VEC3 || a_N.second == GL_FLOAT_VEC4))
            normalAttributeLocation = a_N.first;
        else
            normalAttributeLocation = -1;
        //Locate the vertexColor attribute
        std::pair<int, GLenum> a_C = findAttribute(COLOR_ATTRIBUTE_NAME, this->programId);
        if (a_C.first >= 0 && (a_C.second == GL_FLOAT_VEC3 || a_C.second == GL_FLOAT_VEC4))
            colorAttributeLocation = a_C.first;
        else
            colorAttributeLocation = -1;

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
Sets the pointer from which the ModelView matrix should be loaded from
This pointer is likely provided by the Camera object
*/
void Shaders::setModelViewMatPtr(glm::mat4 const *modelViewMat){
    this->modelviewMat = modelViewMat;
}
/*
Sets the pointer from which the Projection matrix should be loaded from
This pointer is likely provided by the Visualisation object
*/
void Shaders::setProjectionMatPtr(glm::mat4 const *projectionMat){
    this->projectionMat = projectionMat;
}
/*
Call this prior to rendering to enable the program and automatically bind known uniforms and attributes
*/
void Shaders::useProgram(){
    GL_CALL(glUseProgram(this->programId));

    GL_CALL(glBindAttribLocation(this->programId, 0, "in_position"));

    ////glBindAttribLocation(this->programId, 1, "in_normal");
    //this->checkGLError();    //If old shaders
    //Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
    if (this->vertexShaderVersion <= 140 && this->modelviewMat > 0)
    {//If old shaders where gl_ModelViewMatrix is available
        glMatrixMode(GL_MODELVIEW);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*this->modelviewMat)));
    }
    if (this->modelviewMatrixUniformLocation >= 0 && this->modelviewMat > 0)
    {//If modeview matrix location and camera ptr are known
        this->setUniformMatrix4fv(this->modelviewMatrixUniformLocation, glm::value_ptr(*this->modelviewMat));//camera
    }

    //Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
    if (this->vertexShaderVersion <= 140 && this->projectionMat > 0)
    {//If old shaders where gl_ModelViewProjectionMatrix is available
        glMatrixMode(GL_PROJECTION);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*this->projectionMat)));
    }
    if (this->projectionMatrixUniformLocation >= 0 && this->projectionMat > 0)
    {//If projection matrix location and camera ptr are known
        this->setUniformMatrix4fv(this->projectionMatrixUniformLocation, glm::value_ptr(*this->projectionMat));//view frustrum
    }
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
void Shaders::setUniformi(const int location, const int value){
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
void Shaders::setUniformMatrix4fv(const int location, const GLfloat* value){
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
/*
Looks for the '#version xx' tag in the provided shader source and returns the numeric value
@param shaderSource The shader code to detect the version from
@return The detected shader version, 0 if one was not found
*/
unsigned int Shaders::findShaderVersion(const char *shaderSource)
{
    std::cmatch match;
    //\#version ([0-9]+)\ <-versionRegex
    if(std::regex_match(shaderSource, match, this->versionRegex))
        return stoul(match[1]);
    return 0;
}
/*
Attempts to locate the speicified uniform's location and type
@param uniformName The name of the uniform
@param shaderProgram The programId of the shader
@return A pair object whereby the first item is the uniform'd location, and the second item is the type. On failure the first item will be -1
@note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
*/
std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, int shaderProgram)
{
    int result = GL_CALL(glGetUniformLocation(shaderProgram, uniformName));
    if (result > -1)
    {
        GLenum type;
        GLint size;//Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveUniform(shaderProgram, result, 0, 0, &size, &type, 0));
        return std::pair<int, GLenum>(result, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}

/*
Attempts to locate the specified attribute's location and type
@param attributeName The name of the attribute
@param shaderProgram The programId of the shader
@return A pair object whereby the first item is the attribute's location, and the second item is the type. On failure the first item will be -1
@note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
*/
std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, int shaderProgram)
{
    int result = GL_CALL(glGetAttribLocation(shaderProgram, attributeName));
    if (result > -1)
    {
        GLenum type;
        GLint size;//Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveAttrib(shaderProgram, result, 0, 0, &size, &type, 0));
        return std::pair<int, GLenum>(result, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}
