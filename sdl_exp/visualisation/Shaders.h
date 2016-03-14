#ifndef __Shaders_h__
#define __Shaders_h__

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "GLcheck.h"

#include <glm/glm.hpp>
#include <regex>

class Shaders
{
    struct UniformMatrixDetail
    {
        int location=-1; //Uniform location within shader
        glm::mat4 const *matrixPtr=0; //Pointer to the matrix to be loaded
    };
    struct VertexAttributeDetail
    {
        VertexAttributeDetail(int i) : ATTRIB_ARRAY_ID(i) { }
        const int ATTRIB_ARRAY_ID;  //Modern: Which vertex attrib array we store vbo data in.
        int location = -1;          //Modern: Attribute location within shader
        GLuint bufferObject = 0;   //Both: Buffer object containing data
        unsigned int offset = 0;    //Both: Specifies the offset within the buffer object
        unsigned int size = 3;      //Both:   Number of vector elements per attribute (Must be 2, 3 or 4)
        unsigned int stride = 0;    //Both:   Spacing between elements within the array
    };
    //These constants are the names that will be searched for within the shaders
    const char *MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
    const char *PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
    const char *VERTEX_ATTRIBUTE_NAME = "_vertex";
    const char *NORMAL_ATTRIBUTE_NAME = "_normal";
    const char *COLOR_ATTRIBUTE_NAME = "_color";

public:
    Shaders(const char *vertexShaderPath = 0, const char *fragmentShaderPath = 0, const char *geometryShaderPath = 0);
    ~Shaders();

    bool hasVertexShader() const;
    bool hasFragmentShader() const;
    bool hasGeometryShader() const;
    bool getCompileSuccess() const;

    void createShaders();
    bool reload(bool silent = false);
    void useProgram();
    void clearProgram();
    void setUniformi(const int location, const int value);
    void setUniformMatrix4fv(const int location, const GLfloat* value);

    void setModelViewMatPtr(glm::mat4 const *modelViewMat);
    void setProjectionMatPtr(glm::mat4 const *projectionMat);

    void setVertexAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);
    void setVertexNormalAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);
    void setVertexColorAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);

private:
    //Matrix uniform pointers
    UniformMatrixDetail modelview;
    UniformMatrixDetail projection;
    VertexAttributeDetail vertex;
    VertexAttributeDetail normal;
    VertexAttributeDetail color;

    //Shader file paths
    const char *vertexShaderPath;
    const char *fragmentShaderPath;
    const char *geometryShaderPath;
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

    char *loadShaderSource(const char *file);
    void destroyShaders();
    void destroyProgram();
    bool checkShaderCompileError(int shaderId, const char *shaderPath);
    bool checkProgramCompileError(int programId);

    std::regex versionRegex;
    unsigned int findShaderVersion(const char *shaderSource);
    std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, int shaderProgram);
    std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, int shaderProgram);
};

#endif //ifndef __Shaders_h__