#ifndef __Shaders_h__
#define __Shaders_h__

//Define EXIT_ON_ERROR to cause the program to exit when shader compilation fails

#include "GLcheck.h"

#include <glm/glm.hpp>
#include <regex>
#include <vector>
#include <map>
#include <list>
#include <forward_list>

#define NORMALS_SIZE 3
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
    //These constants are the names that will be searched for within the shaders
    const char *MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
    const char *PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
    const char *VERTEX_ATTRIBUTE_NAME = "_vertex";
    const char *NORMAL_ATTRIBUTE_NAME = "_normal";
    const char *COLOR_ATTRIBUTE_NAME = "_color";
    const char *TEXCOORD_ATTRIBUTE_NAME = "_texture";

public:
    struct UniformMatrixDetail
    {
        int location = -1; //Uniform location within shader
        glm::mat4 const *matrixPtr = 0; //Pointer to the matrix to be loaded
    };
    struct VertexAttributeDetail
    {
        VertexAttributeDetail(
            GLenum componentType,
            unsigned int components, 
            unsigned int componentSize
            )
            : componentType(componentType)
            , components(components)
            , componentSize(componentSize)
            , data(0)
            , count(0)
            , vbo(0)
            , location(-1)
            , offset(0)
            , stride(0)
        {}
        GLenum componentType;   //Type
        unsigned int components;          //Number of vector components per attribute (Must be 2, 3 or 4)
        unsigned int componentSize; //Both:
        void *data;                 //Pointer to the attribute data in memory
        unsigned int count;        //Number of attributes contained
        GLuint vbo;                     //Vertex buffer object used
        int location;              //Attribute location within shader
        unsigned int offset;        //Specifies the offset within the vbo
        unsigned int stride;        //Spacing between elements within the array
     };
    struct UniformTextureDetail
    {
        GLuint name;
        GLint location;
        GLenum type;
    };
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

    void setPositionsAttributeDetail(VertexAttributeDetail vad);
    void setNormalsAttributeDetail(VertexAttributeDetail vad);
    void setColorsAttributeDetail(VertexAttributeDetail vad);
    void setTexCoordsAttributeDetail(VertexAttributeDetail vad);

    bool addTextureUniform(GLuint texture, const char *uniformName, GLenum type = GL_TEXTURE_BUFFER);

    bool addDynamicUniform(char *uniformName, GLfloat *array, unsigned int count=1);
    bool addDynamicUniform(char *uniformName, GLint *array, unsigned int count=1);
    bool addStaticUniform(char *uniformName, GLfloat *array, unsigned int count=1);
    bool addStaticUniform(char *uniformName, GLint *array, unsigned int count=1);
private:
    struct DynamicUniformDetail
    {
        GLenum type;
        void *data;
        unsigned int count;
        char *uniformName;
    };
    struct StaticUniformDetail
    {
        GLenum type;
        glm::ivec4 data;
        unsigned int count;
        char *uniformName;
    };
    //Matrix uniform pointers
    UniformMatrixDetail modelview;
    UniformMatrixDetail projection;
    VertexAttributeDetail positions;
    VertexAttributeDetail normals;
    VertexAttributeDetail colors;
    VertexAttributeDetail texcoords;

    //Texture tracking
    std::vector<UniformTextureDetail> textures;

    //Misc uniform tracking
    std::map<GLint, DynamicUniformDetail> dynamicUniforms;
    std::list<DynamicUniformDetail> lostDynamicUniforms;//Ones that went missing after a shader reload
    std::forward_list<StaticUniformDetail> staticUniforms;

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

    std::regex versionRegex;
    unsigned int findShaderVersion(const char *shaderSource);
    std::pair<int, GLenum> Shaders::findUniform(const char *uniformName, const int shaderProgram);
    std::pair<int, GLenum> Shaders::findAttribute(const char *attributeName, const int shaderProgram);
};

#endif //ifndef __Shaders_h__
