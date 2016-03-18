#ifndef __Shaders_h__
#define __Shaders_h__

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "GLcheck.h"

#include <glm/glm.hpp>
#include <regex>
#include <vector>

class Entity;

namespace Stock
{
    namespace Shaders
    {
        struct ShaderSet
        {
            char *vertex;
            char *fragment;
            char *geometry;
        };
        const ShaderSet FLAT{ "../shaders/flat.v", "../shaders/flat.f", 0 };
        const ShaderSet PHONG{ "../shaders/phong.vert", "../shaders/phong.frag", 0 };
        const ShaderSet COLOR{ "../shaders/color.v", "../shaders/color.f", 0 };
    };
};
class Shaders
{
    struct UniformMatrixDetail
    {
        int location=-1; //Uniform location within shader
        glm::mat4 const *matrixPtr=0; //Pointer to the matrix to be loaded
    };
    struct VertexAttributeDetail
    {
        int location = -1;          //Modern: Attribute location within shader
        GLuint bufferObject = 0;    //Both: Buffer object containing data
        unsigned int offset = 0;    //Both: Specifies the offset within the buffer object
        unsigned int size = 3;      //Both:   Number of vector elements per attribute (Must be 2, 3 or 4)
        unsigned int stride = 0;    //Both:   Spacing between elements within the array
    };
    struct UniformTextureDetail
    {
        GLuint name;
        GLint location;
        GLenum type;
    };
    //These constants are the names that will be searched for within the shaders
    const char *MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
    const char *PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
    const char *VERTEX_ATTRIBUTE_NAME = "_vertex";
    const char *NORMAL_ATTRIBUTE_NAME = "_normal";
    const char *COLOR_ATTRIBUTE_NAME = "_color";
    const char *TEXCOORD_ATTRIBUTE_NAME = "_texture";

public:
    Shaders(Stock::Shaders::ShaderSet set);
    Shaders(const char *vertexShaderPath = 0, const char *fragmentShaderPath = 0, const char *geometryShaderPath = 0);
    ~Shaders();

    bool hasVertexShader() const;
    bool hasFragmentShader() const;
    bool hasGeometryShader() const;
    bool getCompileSuccess() const;

    void createShaders();
    bool reload(bool silent = false);
    void useProgram(Entity *e = 0);
    void clearProgram();

    void setModelViewMatPtr(glm::mat4 const *modelViewMat);
    void setProjectionMatPtr(glm::mat4 const *projectionMat);

    void setVertexAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);
    void setNormalAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);
    void setColorAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);
    void setTexCoordAttributeDetail(GLuint bufferObject, unsigned int offset, unsigned int size, unsigned int stride);

    bool addTextureUniform(GLuint texture, const char *uniformName, GLenum type = GL_TEXTURE_BUFFER);

private:
    //Matrix uniform pointers
    UniformMatrixDetail modelview;
    UniformMatrixDetail projection;
    VertexAttributeDetail vertex;
    VertexAttributeDetail normal;
    VertexAttributeDetail color;
    VertexAttributeDetail texcoord;

    //Texture tracking
    std::vector<UniformTextureDetail> textures;
    
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
    bool checkShaderCompileError(const int shaderId, const char *shaderPath);
    bool checkProgramCompileError(const int programId);

    //Private because these must be called after useProgram()
    void setUniformi(const int location, const int value);
    void setUniformMatrix4fv(const int location, const GLfloat* value);

    std::regex versionRegex;
    unsigned int findShaderVersion(const char *shaderSource);
    std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, const int shaderProgram);
    std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, const int shaderProgram);
};

#endif //ifndef __Shaders_h__
