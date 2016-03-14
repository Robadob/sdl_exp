#define  _CRT_SECURE_NO_WARNINGS

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "Shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

/*
Initial value 0, this is just used to auto increment vector attribute array
*/

/*
Constructs a shader program from the provided shader files
@param vertexShaderPath Path to the GLSL vertex shader (nullptr if not required)
@param fragmentShaderPath Path to the GLSL fragment shader (nullptr if not required)
@param geometryShaderPath Path to the GLSL geometry shader (nullptr if not required)
@param camera The camera object to pull the modelView and projection matrices from
*/
Shaders::Shaders(const char *vertexShaderPath, const char *fragmentShaderPath, const char *geometryShaderPath)
    : vertexShaderPath(vertexShaderPath)
    , fragmentShaderPath(fragmentShaderPath)
    , geometryShaderPath(geometryShaderPath)
    , vertexShaderVersion(0)
    , fragmentShaderVersion(0)
    , geometryShaderVersion(0)
    , compileSuccessFlag(true)
    , versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase)
    , modelview{}
    , projection{}
    , vertex{0}
    , normal{1}
    , color{2}
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
Returns whether the last call to createShaders() (via object creation or a call to reload()) was successful
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
            this->modelview.location = u_MV.first;
        else
            this->modelview.location = -1;
        //Locate the projection matrix uniform
        std::pair<int, GLenum> u_PM = findUniform(PROJECTION_MATRIX_UNIFORM_NAME, this->programId);
        if (u_PM.first >= 0 && u_PM.second == GL_FLOAT_MAT4)
            this->projection.location = u_PM.first;
        else
            this->projection.location = -1;
        //Locate the vertexPosition attribute
        std::pair<int, GLenum> a_V = findAttribute(VERTEX_ATTRIBUTE_NAME, this->programId);
        if (a_V.first >= 0 && (a_V.second == GL_FLOAT_VEC3 || a_V.second == GL_FLOAT_VEC4))
            this->vertex.location = a_V.first;
        else
            this->vertex.location = -1;
        //Locate the vertexNormal attribute
        std::pair<int, GLenum> a_N = findAttribute(NORMAL_ATTRIBUTE_NAME, this->programId);
        if (a_N.first >= 0 && (a_N.second == GL_FLOAT_VEC3 || a_N.second == GL_FLOAT_VEC4))
            this->normal.location = a_N.first;
        else
            this->normal.location = -1;
        //Locate the vertexColor attribute
        std::pair<int, GLenum> a_C = findAttribute(COLOR_ATTRIBUTE_NAME, this->programId);
        if (a_C.first >= 0 && (a_C.second == GL_FLOAT_VEC3 || a_C.second == GL_FLOAT_VEC4))
            this->color.location = a_C.first;
        else
            this->color.location = -1;

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
bool Shaders::reload(bool silent){
    if (!silent)
        printf("Reloading Shaders\n");
    this->createShaders();
    return this->compileSuccessFlag;
}
/*
Call this prior to rendering to enable the program and automatically bind known uniforms and attributes
*/
void Shaders::useProgram(){
    GL_CALL(glUseProgram(this->programId));

    //Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
    if (this->vertexShaderVersion <= 140 && this->projection.matrixPtr > 0)
    {//If old shaders where gl_ModelViewProjectionMatrix is available
        glMatrixMode(GL_PROJECTION);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*this->projection.matrixPtr)));
    }
    if (this->projection.location >= 0 && this->projection.matrixPtr > 0)
    {//If projection matrix location and camera ptr are known
        this->setUniformMatrix4fv(this->projection.location, glm::value_ptr(*this->projection.matrixPtr));//view frustrum
    }

    //Set the model view matrix (e.g. gluLookAt, normally provided by the Camera)
    if (this->vertexShaderVersion <= 140 && this->modelview.matrixPtr > 0)
    {//If old shaders where gl_ModelViewMatrix is available
        glMatrixMode(GL_MODELVIEW);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*this->modelview.matrixPtr)));
    }
    if (this->modelview.location >= 0 && this->modelview.matrixPtr > 0)
    {//If modeview matrix location and camera ptr are known
        this->setUniformMatrix4fv(this->modelview.location, glm::value_ptr(*this->modelview.matrixPtr));//camera
    }


    //Don't think we need to use
    //GL_CALL(glBindAttribLocation(this->programId, 0, "in_position"));

    //Set the vertex (location) attribute
    if (this->vertexShaderVersion <= 140 && this->vertex.bufferObject > 0)
    {//If old shaders where gl_Vertex is available
        glEnableClientState(GL_VERTEX_ARRAY);
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->vertex.bufferObject));
        glVertexPointer(this->vertex.size, GL_FLOAT, this->vertex.stride, ((char *)NULL + this->vertex.offset));
    }
    if (this->vertex.location >= 0 && this->vertex.bufferObject > 0)
    {//If vertex attribute location and vbo are known
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->vertex.bufferObject));
        glEnableVertexAttribArray(this->vertex.ATTRIB_ARRAY_ID);
        GL_CALL(glVertexAttribPointer(this->vertex.location, this->vertex.size, GL_FLOAT, GL_FALSE, this->vertex.stride, ((char *)NULL + this->vertex.offset)));
    }

    //Set the vertex normal attribute
    if (this->vertexShaderVersion <= 140 && this->normal.bufferObject > 0)
    {//If old shaders where gl_Vertex is available
        glEnableClientState(GL_NORMAL_ARRAY);
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normal.bufferObject));
        glNormalPointer(GL_FLOAT, this->normal.stride, ((char *)NULL + this->normal.offset));
    }
    if (this->normal.location >= 0 && this->normal.bufferObject > 0)
    {//If vertex attribute location and vbo are known
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normal.bufferObject));
        glEnableVertexAttribArray(this->normal.ATTRIB_ARRAY_ID);
        GL_CALL(glVertexAttribPointer(this->normal.location, this->normal.size, GL_FLOAT, GL_FALSE, this->normal.stride, ((char *)NULL + this->normal.offset)));
    }

    //Set the vertex color attribute
    if (this->vertexShaderVersion <= 140 && this->color.bufferObject > 0)
    {//If old shaders where gl_Vertex is available
        glEnableClientState(GL_COLOR_ARRAY);
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->color.bufferObject));
        glColorPointer(this->color.size, GL_FLOAT, this->color.stride, ((char *)NULL + this->color.offset));
    }
    if (this->color.location >= 0 && this->color.bufferObject >= 0)
    {//If vertex attribute location and vbo are known
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->color.bufferObject));
        glEnableVertexAttribArray(this->color.ATTRIB_ARRAY_ID);
        GL_CALL(glVertexAttribPointer(this->color.location, this->color.size, GL_FLOAT, GL_FALSE, this->color.stride, ((char *)NULL + this->color.offset)));
    }
}
/*
Disables the currently active shader progam and attributes attached to this shader
*/
void Shaders::clearProgram(){
    //Vertex location
    if (this->vertexShaderVersion <= 140 && this->vertex.bufferObject > 0)
    {//If old shaders where gl_Vertex is available
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    if (this->vertex.location >= 0 && this->vertex.bufferObject > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->vertex.ATTRIB_ARRAY_ID);
    }
    //Vertex normal
    if (this->vertexShaderVersion <= 140 && this->normal.bufferObject > 0)
    {//If old shaders where gl_Normal is available
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if (this->normal.location >= 0 && this->normal.bufferObject > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->normal.ATTRIB_ARRAY_ID);
    }
    //Vertex color
    if (this->vertexShaderVersion <= 140 && this->color.bufferObject > 0)
    {//If old shaders where gl_Color is available
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (this->color.location >= 0 && this->color.bufferObject > 0)
    {//If vertex attribute location and vbo are known
        glDisableVertexAttribArray(this->color.ATTRIB_ARRAY_ID);
    }
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
char* Shaders::loadShaderSource(const char* file){
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
bool Shaders::checkShaderCompileError(int shaderId, const char* shaderPath){
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
    if (std::regex_search(shaderSource, match, this->versionRegex))
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
/*
Sets the pointer from which the ModelView matrix should be loaded from
This pointer is likely provided by the Camera object
*/
void Shaders::setModelViewMatPtr(glm::mat4 const *modelViewMat){
    this->modelview.matrixPtr = modelViewMat;
}
/*
Sets the pointer from which the Projection matrix should be loaded from
This pointer is likely provided by the Visualisation object
*/
void Shaders::setProjectionMatPtr(glm::mat4 const *projectionMat){
    this->projection.matrixPtr = projectionMat;
}
/*
Stores the details necessary for passing vertex (location) attributes to the shader via the modern method
@param bufferObject The buffer object containing the attribute data
@param offset The byte offset within the buffer that the data starts
@param size The number of components per attribute (either 2, 3 or 4)
@param stride The byte offset between consecutive attributes
*/
void Shaders::setVertexAttributeDetail(GLuint bufferObject, unsigned int offset = 0, unsigned int size=3, unsigned int stride=0)
{
    this->vertex.bufferObject = bufferObject;
    this->vertex.offset = offset;
    this->vertex.size = size;
    this->vertex.stride = stride;
}
/*
Stores the details necessary for passing vertex normal attributes to the shader via the modern method
@param bufferObject The buffer object containing the attribute data
@param offset The byte offset within the buffer that the data starts
@param size The number of components per attribute (either 2, 3 or 4)
@param stride The byte offset between consecutive attributes
*/
void Shaders::setVertexNormalAttributeDetail(GLuint bufferObject, unsigned int offset = 0, unsigned int size = 3, unsigned int stride = 0)
{
    this->normal.bufferObject = bufferObject;
    this->normal.offset = offset;
    this->normal.size = size;
    this->normal.stride = stride;
}
/*
Stores the details necessary for passing vertex normal attributes to the shader via the modern method
@param bufferObject The buffer object containing the attribute data
@param offset The byte offset within the buffer that the data starts
@param size The number of components per attribute (either 2, 3 or 4)
@param stride The byte offset between consecutive attributes
*/
void Shaders::setVertexColorAttributeDetail(GLuint bufferObject, unsigned int offset = 0, unsigned int size = 3, unsigned int stride = 0)
{
    this->color.bufferObject = bufferObject;
    this->color.offset = offset;
    this->color.size = size;
    this->color.stride = stride;
}
