#ifndef __Shaders_h__
#define __Shaders_h__


#include "ShaderCore.h"

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
        const ShaderSet FIXED_FUNCTION{ 0, 0, 0 };
        const ShaderSet FLAT{ "../shaders/flat.vert", "../shaders/flat.frag", 0 };
        const ShaderSet PHONG{ "../shaders/phong.vert", "../shaders/phong.frag", 0 };
        const ShaderSet COLOR{ "../shaders/color.vert", "../shaders/color.frag", 0 };
        const ShaderSet TEXTURE{ "../shaders/texture.vert", "../shaders/texture.frag", 0 };
        const ShaderSet SKYBOX{ "../shaders/skybox.vert", "../shaders/skybox.frag", 0 };
		const ShaderSet INSTANCED{ "../shaders/instanced.vert", "../shaders/flat.frag", 0 };
		const ShaderSet TEXT{ "../shaders/texture.vert", "../shaders/text.frag", 0 };
        const ShaderSet SPRITE2D{ "../shaders/texture.vert", "../shaders/sprite2d.frag", 0 };
        const ShaderSet BILLBOARD{ "../shaders/billboard.vert", "../shaders/particle.frag", 0 };
    };
};
/*
Abstracts compilation of Shaders, and attempts to automatically bind uniforms and attributes.
Each Shaders object is 'bound' to a single entity, so create a 2nd if you wish to use the same shaders with a seperate entity.
*/
class Shaders : private ShaderCore
{
public:
	//These constants are the names that will be searched for when loading shaders
	static const char *MODELVIEW_MATRIX_UNIFORM_NAME;//="_modelViewMat";
	static const char *PROJECTION_MATRIX_UNIFORM_NAME;//="_projectionMat";
	static const char *MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME;// "_modelViewProjectionMat";
	static const char *VERTEX_ATTRIBUTE_NAME;// = "_vertex";
	static const char *NORMAL_ATTRIBUTE_NAME;// = "_normal";
	static const char *COLOR_ATTRIBUTE_NAME;// = "_color";
	static const char *TEXCOORD_ATTRIBUTE_NAME;// = "_texCoords";

public:
    struct UniformMatrixDetail
    {
        UniformMatrixDetail(int location = -1, glm::mat4 const *matrixPtr = 0)
            :location(location), matrixPtr(matrixPtr) { }
        int location; //Uniform location within shader
        glm::mat4 const *matrixPtr; //Pointer to the matrix to be loaded
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
    Shaders(Stock::Shaders::ShaderSet set);
    Shaders(const char *vertexShaderPath = 0, const char *fragmentShaderPath = 0, const char *geometryShaderPath = 0);
    ~Shaders();

    bool hasVertexShader() const;
    bool hasFragmentShader() const;
    bool hasGeometryShader() const;
    //bool getCompileSuccess() const;
    //int getProgram();

    //void createShaders();
    //bool reload(bool silent = false);
    void useProgram(Entity *e = 0);
	void useProgram(const glm::mat4 *mv, const glm::mat4 *proj);
	//void destroyProgram();

    void setModelViewMatPtr(glm::mat4 const *modelViewMat);
    void setProjectionMatPtr(glm::mat4 const *projectionMat);

    void setPositionsAttributeDetail(VertexAttributeDetail vad);
    void setNormalsAttributeDetail(VertexAttributeDetail vad);
    void setColorsAttributeDetail(VertexAttributeDetail vad);
    void setTexCoordsAttributeDetail(VertexAttributeDetail vad);

    void setColor(glm::vec3 color);
    void setColor(glm::vec4 color);

private:
	void _clearProgram() override;
	void _useProgram();
    //Matrix uniform pointers
    UniformMatrixDetail modelview;
    UniformMatrixDetail projection;
    int modelviewprojection;
    VertexAttributeDetail positions;
    VertexAttributeDetail normals;
    VertexAttributeDetail colors;
    VertexAttributeDetail texcoords;
    int colorUniformLocation;
    int colorUniformSize;


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

    void destroyShaders();
};

#endif //ifndef __Shaders_h__
