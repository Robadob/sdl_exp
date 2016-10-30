#ifndef __Shaders_h__
#define __Shaders_h__


#include "ShaderCore.h"
#include <glm/glm.hpp>

#define NORMALS_SIZE 3

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
 * Abstracts compilation of Shaders, and attempts to automatically bind provided uniforms and vertex attributes on useProgram()
 * Bindings are tied to the specific shader, if you wish to use the same shader for 2 entities it might be best to make a 2nd instance
 * @note There is an implementation specific limit of around 16 vertex attribute bindings you can have to any shader, this not enforced by this class
 * @note Floating point attribute bindings do not automatically normalise values (the parameter is always passed as false)
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
	static const char *NORMAL_MATRIX_UNIFORM_NAME;// = "_normalMat";
	static const char *CAMERA_MATRIX_UNIFORM_NAME;// = "_cameraMat";
	static const char *VERTEX_ATTRIBUTE_NAME;// = "_vertex";
	static const char *NORMAL_ATTRIBUTE_NAME;// = "_normal";
	static const char *COLOR_ATTRIBUTE_NAME;// = "_color";
	static const char *TEXCOORD_ATTRIBUTE_NAME;// = "_texCoords";
	static const char *PREV_MODELVIEW_MATRIX_UNIFORM_NAME;// = "_prevModelViewMat";
	/**
	 * This structure represents the details necessary to correctly bind a uniform matrix (e.g. model view/projection)
	 */
    struct UniformMatrixDetail
    {
		UniformMatrixDetail(int location = -1, const glm::mat4 *matrixPtr = nullptr)
            :location(location), matrixPtr(matrixPtr) { }
		/**
		 * This value is set internally when we detect that your shader has a suitable uniform location
		 */
        int location;
		/**
		 * Const pointer to the matrix to be loaded
		 */
		const glm::mat4 *matrixPtr;
	};
	/**
	 * This structure represents the details necessary to correctly bind a vertex attribute (e.g. vertex positions, normals, tex coords)
	 * Most of the member vars provide a specification of how the array of vertex attributes is stored in the vbo
	 * Each vertex attribute should be a vector [element of the array], and has 2-4 components
	 * Multiple vertex attributes can share a vbo, with their data either packed one after the other, or interleaved.
	 */
    struct VertexAttributeDetail
    {
		/**
		 * Other values should be set after creation
		 * location can be ignored as this is used internally
		 */
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
		/**
		 * Underlying component type expressed as GLenum
		 * e.g. Most cases will be float4/glm::vec4 which are GL_FLOAT
		 * Options: GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT
		 */
        GLenum componentType;
		/**
		 * Number of vector components (Must be 2,3 or 4)
		 */
        unsigned int components;    //Number of vector components per attribute (Must be 2, 3 or 4)
		/**
		* Size of the component as specified in componentType
		* e.g. sizeof(float)
		*/
        unsigned int componentSize; //Both:
		/**
		 * Pointer to the attribute data in memory
		 */
        void *data;
		/**
		 * Number of attributes in the array
		 */
        unsigned int count;
		/**
		 * VBO name as returned by glGenBuffers()
		 */
        GLuint vbo;
		/**
		 * This value is internally set when the Shader object detects a suitable binding location
		 */
        int location; 
		/**
		 * The offset from the start of the vbo which attribute data begins
		 * @note You store multiple vertex attribute arrays in the same vbo
		 */
        unsigned int offset;
		/**
		 * Spacing between elements within the array
		 * @note This is value is 0 unless the data is interleaved
		 */
        unsigned int stride;
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
	/**
	 * Free the shader program
	 */
	~Shaders();
	/**
	 * @return True if this shader object has a vertex shader
	 */
	bool hasVertexShader() const;
	/**
	 * @return True if this shader object has a fragment shader
	 */
	bool hasFragmentShader() const;
	/**
	 * @return True if this shader object has a geometry shader
	 */
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
	/**
	 * Binds the named fragment shader output attribute to the specified framebuffer colour attachment point
	 * @param attachmentPoint The GL_COLOR_ATTACHMENT value (likely in the range 0-7)
	 * @param name the name of the data output as specified within the frag shader
	 * @return Returns true if the re-linking of the shader was successful
	 * @note If the framebuffer has only 1 color attachment, calling this is unnecessary.
	 * @note Alternatively, you can hardcode attachment point 'n' in the shader, by preceding the output declaration with layout(location = n)
	 * @note This function re-links the shader program, so may cause the shader to fail
	 */
	bool setFragOutAttribute(GLuint attachmentPoint, const char *name);
	/**
	 * Subclass of vertex attribute detail for generic vertex attributes
	 * This attaches the name of the vertex attribute within the shader source
	 */
	struct GenericVAD :public VertexAttributeDetail
	{
		GenericVAD(
			VertexAttributeDetail vad,
			const char* attributeName
			)
			: VertexAttributeDetail(vad)
			, attributeName(attributeName)
		{}
		/**
		 * Identifier of the vertex attribute within the shader source
		 */
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
	 * Disables the vertex attributes attached to this shader
	 * @note Uses glDisableClientState() and glDisableVertexAttribArray()
	 */
	void _clearProgram() override;
	/**
	 * Enables the necessary vertex attribute arrays when the shader is required
	 * @note Called by ShaderCore::useProgram()	 
	 * @note In future this could be improved with vertex array object's to store attribute configs at shader setup
	 */
	void _useProgram() override;
	/**
	 * Compiles the shader sources specified at the objects creation
	 * @param t_shaderProgram The shader program the shader should be attached to.
	 * @note This function simply calls return compileShader()&&compileShader().. with each init list of source files
	**/
	bool _compileShaders(const GLuint t_shaderProgram)override;
	/**
	 * Binds Shaders class specific bindings to the shader program
	 * These are primarily vertex attributes and modelview/projection matrices
	 * Textures, buffers and misc uniforms are handled by ShaderCore::setupBindings() as these are also used by ComputeShader
	 */
	void _setupBindings() override;
	/**
	 * Information for binding the modelview matrix
	 */
    UniformMatrixDetail modelview;
	/**
	 * Information for binding the projection matrix
	 */
    UniformMatrixDetail projection;
	/**
	 * When positive this variable holds the location of the (combined) modelviewprojection matrix in the shader
	 */
	int modelviewprojection;
	/**
	 * When positive this vairable holds the location of the camera matrix in the shader
	 * This is simply the modelview mat provided by the camera, before transformations are applied
	 */
	int cameraMatLoc;
	/**
	 * When positive this vairable holds the location of the normal matrix in the shader
	 */
	int normalMatLoc;
	/**
	 * Cache's the previous frames modelview mat to be passed if _prevModelViewMat is required
	 * @note Used for producing velocity map's
	 */
	glm::mat4 prevModelview;
	int prevModelviewUniformLocation;
	/**
	 * When !nullptr, points to a vector containing a rotation applied to the modelview matrix before binding
	 */
	const glm::vec4 *rotationPtr;
	/**
	* When !nullptr, points to a vector containing a translation applied to the modelview matrix before binding
	*/
	const glm::vec3 *translationPtr;
	/**
	 * Holds information for binding the vertex positions attribute
	 */
	VertexAttributeDetail positions;
	/**
	* Holds information for binding the vertex normals attribute
	*/
	VertexAttributeDetail normals;
	/**
	* Holds information for binding the vertex colours attribute
	*/
	VertexAttributeDetail colors;
	/**
	* Holds information for binding the vertex texture coordinates attribute
	*/
    VertexAttributeDetail texcoords;
	/**
	 * If the default color uniform identifier is found within the shader, it's location is stored here
	 */
    int colorUniformLocation;
	/**
	 * The number of components in the colour uniform that was located
	 */
    int colorUniformSize;
	/**
	 * The data to be passed to the colour uniform
	 * This is static data, but we store the value so we can maintain it on reload
	 */
	glm::vec4 colorUniformValue;
	/**
	 * The color attachment point to bind to each output identifier within the frag shader
	 * key: attachment point
	 * value: output variable identifier
	 */
	std::map<GLuint, std::string> fragShaderOutputLocations;
	/**
	 * The path to the files which form the vertex shader
	 * @note Used on shader reload
	 */
	std::vector<const std::string> *vertexShaderFiles;
	/**
	* The path to the files which form the fragment shader
	* @note Used on shader reload
	*/
	std::vector<const std::string> *fragmentShaderFiles;
	/**
	* The path to the files which form the geometry shader
	* @note Used on shader reload
	*/
	std::vector<const std::string> *geometryShaderFiles;
	/**
	 * The GLSL version used within the vertex shader source
	 * @note This value is detected from the #version define in the source file
	 */
	int vertexShaderVersion;
	/**
	* The GLSL version used within the fragment shader source
	* @note This value is detected from the #version define in the source file
	*/
	int fragmentShaderVersion;
	/**
	* The GLSL version used within the geometry shader source
	* @note This value is detected from the #version define in the source file
	*/
    int geometryShaderVersion;
};

#endif //ifndef __Shaders_h__
