#ifndef __Shaders_h__
#define __Shaders_h__


#include "ShaderCore.h"
#include <glm/glm.hpp>

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
        const ShaderSet FIXED_FUNCTION{ nullptr, nullptr, nullptr };
        const ShaderSet FLAT{ "../shaders/flat.vert", "../shaders/flat.frag", nullptr };
        const ShaderSet PHONG{ "../shaders/phong.vert", "../shaders/phong.frag", nullptr };
        const ShaderSet COLOR{ "../shaders/color.vert", "../shaders/color.frag", nullptr };
        const ShaderSet TEXTURE{ "../shaders/texture.vert", "../shaders/texture.frag", nullptr };
        const ShaderSet SKYBOX{ "../shaders/skybox.vert", "../shaders/skybox.frag", nullptr };
		const ShaderSet INSTANCED{ "../shaders/instanced.vert", "../shaders/flat.frag", nullptr };
		const ShaderSet TEXT{ "../shaders/texture.vert", "../shaders/text.frag", nullptr };
        const ShaderSet SPRITE2D{ "../shaders/texture.vert", "../shaders/sprite2d.frag", nullptr };
        const ShaderSet BILLBOARD{ "../shaders/billboard.vert", "../shaders/particle.frag", nullptr };
    };
};
/**
 * Abstracts compilation of Shaders, and attempts to automatically bind uniforms and attributes.
 * Each Shaders object is 'bound' to a single entity, so create a 2nd if you wish to use the same shaders with a seperate entity.
 * @todo Confirm that we do need re-call glVertexAttribPointer on shader re-use, or can we just bind it once
 * @todo Switch all pointers to weak_ptr
*/
class Shaders : public ShaderCore
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

    struct UniformMatrixDetail
    {
		UniformMatrixDetail(int location = -1, const glm::mat4 *matrixPtr = nullptr)
            :location(location), matrixPtr(matrixPtr) { }
        int location; //Uniform location within shader
		const glm::mat4 *matrixPtr; //Pointer to the matrix to be loaded
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
            , data(nullptr)
            , count(0)
            , vbo(0)
            , location(-1)
            , offset(0)
            , stride(0)
        {}
        GLenum componentType;       //Type
        unsigned int components;    //Number of vector components per attribute (Must be 2, 3 or 4)
        unsigned int componentSize; //Both:
        void *data;                 //Pointer to the attribute data in memory
        unsigned int count;         //Number of attributes contained
        GLuint vbo;                 //Vertex buffer object used
        int location;               //Attribute location within shader
        unsigned int offset;        //Specifies the offset within the vbo
        unsigned int stride;        //Spacing between elements within the array
     };
	/**
	 * Constructs a shader object from one of the stock shader sets
	 * @param set The shader set to create
	 */
	Shaders(Stock::Shaders::ShaderSet set);
	/*
	 * Constructs a shader program from the provided shader files
	 * @param vertexShaderFiles Path to the GLSL vertex shader (nullptr if not required)
	 * @param fragmentShaderFiles Path to the GLSL fragment shader (nullptr if not required)
	 * @param geometryShaderFiles Path to the GLSL geometry shader (nullptr if not required)
	 * @note You must provide atleast 1 shader path, however it can be of any of the 3 types
	 */
	Shaders(const char *vertexShaderPath = nullptr, const char *fragmentShaderPath = nullptr, const char *geometryShaderPath = nullptr);
	/*
	* Constructs a shader program from the provided shader files
	* Use this constructor if you wish to constructor a shader from multiple files (e.g. a common functions file)
	* @param vertexShaderFiles Path to the GLSL vertex shader ({} if not required)
	* @param fragmentShaderFiles Path to the GLSL fragment shader ({} if not required)
	* @param geometryShaderFiles Path to the GLSL geometry shader ({} if not required)
	* @note You must provide atleast 1 shader path, however it can be of any of the 3 types
	*/
	Shaders(std::initializer_list <const char *> vertexShaderPath, std::initializer_list <const char *> fragmentShaderPath = {}, std::initializer_list <const char *> geometryShaderPath = {});
    ~Shaders();

    bool hasVertexShader() const;
    bool hasFragmentShader() const;
    bool hasGeometryShader() const;
    
	/**
	 * Sets the pointer from which the ModelView matrix should be loaded from
	 * @param modelViewMat A pointer to the projectionMatrix to be tracked
	 * @note This pointer is likely provided by a Camera subclass
	 */
	inline void setModelViewMatPtr(const glm::mat4  *modelViewMat){ this->modelview.matrixPtr = modelViewMat; }
	/**
	 * Sets the pointer from which the Projection matrix should be loaded from
	 * @param projectionMat A pointer to the projectionMatrix to be tracked
	 * @note This pointer is likely provided by the Visualisation object
	 */
	inline void setProjectionMatPtr(const glm::mat4 *projectionMat){ this->projection.matrixPtr = projectionMat; }
	/**
	* Sets the pointer which will apply a rotation to the ModelView matrix, rotating items rendered by this shader
	* @param rotationPtr A pointer to the rotation will be tracked
	* @note Setting this pointer to nullptr will disable rotation
	* @note rotationPtr.xyz is the axis to rotate about
	* @note rotationPtr.w is the rotation angle in degrees
	*/
	inline void setRotationPtr(glm::vec4 const *rotationPtr){ this->rotationPtr = rotationPtr; }
	/**
	* Sets the pointer which will apply a translation to the ModelView matrix, rotating items rendered by this shader
	* @param translationPtr A pointer to the projectionMatrix to be tracked
	* @note Setting this pointer to nullptr will disable translation
	*/
	inline void setTranslationPtr(glm::vec3 const *translationPtr){ this->translationPtr = translationPtr; }
	/**
	* Stores the details necessary for passing vertex position attributes to the shader via the modern method
	* @param vad The VertexAttributeDetail object containing the attribute data
	*/
	void setPositionsAttributeDetail(VertexAttributeDetail vad);
	/**
	* Stores the details necessary for passing vertex normal attributes
	* @param vad The VertexAttributeDetail object containing the attribute data
	*/
	void setNormalsAttributeDetail(VertexAttributeDetail vad);
	/**
	* Stores the details necessary for passing vertex color attributes to the shader
	* @param vad The VertexAttributeDetail object containing the attribute data
	*/
	void setColorsAttributeDetail(VertexAttributeDetail vad);
	/**
	* Stores the details necessary for passing vertex texture attributes to the shader
	* @param vad The VertexAttributeDetail object containing the attribute data
	*/
	void setTexCoordsAttributeDetail(VertexAttributeDetail vad);
	struct GenericVAD :public VertexAttributeDetail
	{
		GenericVAD(
			VertexAttributeDetail vad,
			const char* attributeName
			)
			: VertexAttributeDetail(vad)
			, attributeName(attributeName)
		{}
		const char* attributeName;
	};
	/**
	* Holds additional information necessary for tracking generic vertex attributes
	* Key: Assigned generic buffer ID 
	* Value: Other details necessary for binding
	*/
	std::list<GenericVAD> gvads;
	/**
	* Holds generic vertex attributes that were not found within the shader
	* or went missing after a shader reload
	*/
	std::list<GenericVAD> lostGvads;
	bool addGenericAttributeDetail(const char* attributeName, VertexAttributeDetail vad);

	inline void clearModelViewMatPtr(){ this->modelview.matrixPtr = nullptr; }
	inline void clearProjectionMatPtr(){ this->projection.matrixPtr = nullptr; }
	inline void clearPositionsAttributeDetail()
	{
		VertexAttributeDetail vad(GL_FLOAT, 3, sizeof(float));
		vad.location = this->positions.location;
		this->positions = vad;
	}
	inline void clearNormalsAttributeDetail()
	{
		VertexAttributeDetail vad(GL_FLOAT, NORMALS_SIZE, sizeof(float));
		vad.location = this->normals.location;
		this->normals = vad;
	}
	inline void clearColorsAttributeDetail()
	{
		VertexAttributeDetail vad(GL_FLOAT, 3, sizeof(float));
		vad.location = this->colors.location;
		this->colors = vad;
	}
	inline void clearTexCoordsAttributeDetail()
	{
		VertexAttributeDetail vad(GL_FLOAT, 2, sizeof(float));
		vad.location = this->texcoords.location;
		this->texcoords = vad;
	}
	bool removeGenericAttributeDetail(const char* attributeName);
	/**
	* Sets the color uniform
	* @param color The RGB value of the color
	*/
	void setColor(glm::vec3 color);
	/**
	* Sets the color uniform
	* @param color The RGBA value of the color
	*/
    void setColor(glm::vec4 color);

private:
	/**
	 * Disables the attributes attached to this shader
	 */
	void _clearProgram() override;
	void _useProgram() override;
	/**
	 * Compiles the shader sources specified at the objects creation
	 * @param t_shaderProgram The shader program the shader should be attached to.
	 * @note This function simply calls return compileShader()&&compileShader().. with each init list of source files
	**/
	bool _compileShaders(const GLuint t_shaderProgram)override;
	void _setupBindings() override;
    //Matrix uniform pointers
    UniformMatrixDetail modelview;
    UniformMatrixDetail projection;
    int modelviewprojection;
	const glm::vec4 *rotationPtr;
	const glm::vec3 *translationPtr;
    VertexAttributeDetail positions;
    VertexAttributeDetail normals;
    VertexAttributeDetail colors;
    VertexAttributeDetail texcoords;
    int colorUniformLocation;
    int colorUniformSize;
	glm::vec4 colorUniformValue;
    //Shader file paths
	std::initializer_list <const char *> vertexShaderFiles;
	std::initializer_list <const char *> fragmentShaderFiles;
	std::initializer_list <const char *> geometryShaderFiles;
    //Shader module IDs
    int vertexShaderId;
    int fragmentShaderId;
    int geometryShaderId;
    //Detected shader versions
    int vertexShaderVersion;
    int fragmentShaderVersion;
    int geometryShaderVersion;
};

#endif //ifndef __Shaders_h__
