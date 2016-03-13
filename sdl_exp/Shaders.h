#ifndef __Shaders_h__
#define __Shaders_h__

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "GLcheck.h"

#include <glm\glm.hpp>
#include <regex>

class Shaders
{
    const char *MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
    const char *PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
    const char *VERTEX_ATTRIBUTE_NAME = "_vertex";
    const char *NORMAL_ATTRIBUTE_NAME = "_normal";
    const char *COLOR_ATTRIBUTE_NAME = "_color";

public:
    Shaders(char *vertexShaderPath = 0, char *fragmentShaderPath = 0, char *geometryShaderPath = 0);
    ~Shaders();

    bool hasVertexShader() const;
    bool hasFragmentShader() const;
    bool hasGeometryShader() const;
    bool getCompileSuccess() const;

    void createShaders();
    bool reloadShaders(bool silent = false);
    void useProgram();
    void clearProgram();
    void setUniformi(const int location, const int value);
    void setUniformMatrix4fv(const int location, const GLfloat* value);

    void setModelViewMatPtr(glm::mat4 const *modelViewMat);
    void setProjectionMatPtr(glm::mat4 const *projectionMat);

private:
    //Uniform and Attrib locations
    int modelviewMatrixUniformLocation;
    int projectionMatrixUniformLocation;
    int vertexAttributeLocation;
    int normalAttributeLocation;
    int colorAttributeLocation;
    //Matrix uniform pointers
    glm::mat4 const *modelviewMat;
    glm::mat4 const *projectionMat;

    //Shader file paths
    char *vertexShaderPath;
    char *fragmentShaderPath;
    char *geometryShaderPath;
    //Shader module IDs
    int vertexShaderId;
    int fragmentShaderId;
    int geometryShaderId;
    //Detected shader versions
    unsigned int vertexShaderVersion;
    unsigned int fragmentShaderVersion;
    unsigned int geometryShaderVersion;
    //Shader program ID
    int programId;
    bool compileSuccessFlag;

    char *loadShaderSource(char *file);
    void destroyShaders();
    void destroyProgram();
    bool checkShaderCompileError(int shaderId, char *shaderPath);
    bool checkProgramCompileError(int programId);

    std::regex versionRegex;
    unsigned int findShaderVersion(const char *shaderSource);
    std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, int shaderProgram);
    std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, int shaderProgram);

};

#endif //ifndef __Shaders_h__