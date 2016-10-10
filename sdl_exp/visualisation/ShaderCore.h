#ifndef __ShaderCore_h__
#define __ShaderCore_h__

#include "GLcheck.h"
#include "Reloadable.h"
#include <vector>
#include <map>
#include <list>
#include <forward_list>
#include <glm/vec4.hpp>
//TODO replace uniform bindings pointers with weak ptrs
//TODO manage texture uniforms better so they can be removed (currently bufferId is simply incremeneted, what if we delete and re add tex's?)
//Add removeUniform functionalities
/**
 *
 */
class ShaderCore : public Reloadable
{
private:
	/**
	 * Whether the shader object is ready to be used
	 */
	bool compileSuccessFlag;
	/**
	 * OpenGLs reference to the shader
	 * Negative values are invalid, meaning the shader is not ready
	 */
	int programId;
	/**
	 * Whether the shader object should exit program execution if a shader compilation/linking fails
	 */
	bool exitOnError;
	/**
	 * A constant string to identify the particular shader instance
	 */
	const char* shaderTag;
protected:
	/**
	 * Construtor, initialises common values
	 * @note Does not setup the shader object (reload() does that)
	 */
	ShaderCore(const char *shaderTag);
	/**
	 * There is no reason to ever have a pointer to ShaderCore (the subclasses are too distinct, are used via their own methods)
	 * Therefore no reason to delete one directly, hence protected
	 */
	virtual ~ShaderCore();
public:
	/**
     * Reloads the shader source from file, recompiles it and rebinds all bound items
	 * Calls _reload() on subclass to load subclass specific items
	 * @see _reload()
	 */
	void reload() final;
	/**
	 * Returns OpenGLs reference to the shader
	 * @return The OpenGL program id of the shader
	 * @note Negative values mean the shader is not currently registered with OpenGL
	 */
	int getProgram() const { return programId; }
	/**
	* Disables the currently active shader program and asks subclasses to clear any enabled client states or attribute arrays
	* @see _clearProgram()
	*/
	void clearProgram();
	/**
	* Calls glUseProgram(GLuint) and binds all dynamic uniforms/textures
	* @see _useProgram()
	*/
	void useProgram();
	/**
	 * Returns whether the shader is compiled and ready
	 * @return True if the shader is ready to be used, else false
	 */
	bool isReady() const { return programId>=0; }
	/**
	 * Deletes the shader program
	 * @see glDeleteProgram(GLuint)
	 */
	void destroyProgram();
	/**
	 * Binds a texture buffer to be loaded when useProgram() is called
	 * @param texture The name of the texture (as returned by glGenTexture())
	 * @param uniformName The name of the uniform within the shader this texture should be bound to
	 * @param type The type of texture being bound (e.g. GL_TEXTURE2D)
	 * @return The texture unit the texture has been bound to, on failure (due to no texture units remaining) -1
	 */
	int addTextureUniform(GLuint texture, char *uniformName, GLenum type = GL_TEXTURE_BUFFER);
	/**
 	 * Remembers a pointer to an array of upto 4 floats that will be updated everytime useProgram() is called on this Shaders object
     * @param uniformName The name of the uniform within the shader
	 * @param arry A pointer to the array of floats
	 * @param count The number of floats provided in the array (a maximum of 4)
	 * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
	 * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
	 * @see addDynamicUniform(const char *,const GLint *, unsigned int)
	 */
	bool addDynamicUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1);
	/**
	 * Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
	 * @param uniformName The name of the uniform within the shader
	 * @param arry A pointer to the array of integers
	 * @param count The number of integers provided in the array (a maximum of 4)
	 * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
	 * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
	 * @see addDynamicUniform(const char *,const GLfloat *, unsigned int)
	 */
	bool addDynamicUniform(const char *uniformName, const GLint *arry, unsigned int count = 1);
	/**
	 * Sets the
	 * Remembers a pointer to an array of upto 4 floats that will be updated everytime useProgram() is called on this Shaders object
	 * @param uniformName The name of the uniform within the shader
	 * @param arry A pointer to the array of floats
	 * @param count The number of floats provided in the array (a maximum of 4)
	 * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
	 * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
	 */
	bool addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1);
	/**
	 * Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
	 * @param uniformName The name of the uniform within the shader
	 * @param arry A pointer to the array of integers
	 * @param count The number of integers provided in the array (a maximum of 4)
	 * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
	 * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
	 */
	bool addStaticUniform(const char *uniformName, const GLint *arry, unsigned int count = 1);
private:
	/**
	 * Holds all the necessary information for binding dynamic uniform int and float vectors
	 */
	struct DynamicUniformDetail
	{
		/**
		 * GL_INT or GL_FLOAT
		 */
		const GLenum type;
		/**
		 * Pointer to the data we want to copy to the shader uniform
		 */
		const void *data;
		/**
		 * Number of vector components
		 */
		const unsigned int count;
		/**
		 * Identifier of the uniform within the shader source
		 */
		const char *uniformName;
	};
	/**
	 * Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
	 * @see addDynamicUniform(const char *,const GLfloat *, unsigned int)
	 * @see addDynamicUniform(const char *,const GLint *, unsigned int) 
	 */
	bool addDynamicUniform(DynamicUniformDetail d);
	/**
	 * Holds dynamic uniforms that have been found within the shader
	 * Key: Uniform location within shader
	 * Val: Additional information necessary for setting the uniform
	 */
	std::map<GLint, DynamicUniformDetail> dynamicUniforms;
	/**
	* Holds dynamic uniforms that were not found within the shader
	*/
	std::list<DynamicUniformDetail> lostDynamicUniforms;//Ones that went missing after a shader reload
	/**
	* Holds all the necessary information for binding static uniform int and float vectors
	*/
	struct StaticUniformDetail
	{
		/**
		* GL_INT or GL_FLOAT
		*/
		const GLenum type;
		/**
		* Data to be stored in the shader uniform
		* @note components greater than count-1 are likely to contain garbage data
		* @note this may actually contain a glm::vec4 if type==GL_FLOAT
		*/
		const glm::ivec4 data;
		/**
		* Number of vector components
		*/
		const unsigned int count;
		/**
		* Identifier of the uniform within the shader source
		*/
		const char *uniformName;
	};
	/**
	* Holds information necessary for setting the static uniforms to be bound to the shader
	*/
	std::forward_list<StaticUniformDetail> staticUniforms;
	/**
	* Holds all the additional information for binding texture uniforms
	*/
	struct UniformTextureDetail
	{
		/**
		* The name of the texture (as returned by glGenTexture())
		*/
		const GLuint name;
		/**
		* The type of texture (e.g. GL_TEXTURE2D)
		*/
		const GLint bufferId;
		/**
		 * The type of texture (e.g. GL_TEXTURE2D)
		 */
		const GLenum type;
	};
	/**
	 * Holds additional information necessary for tracking texture uniforms
	 * Items in this vector have a counterpart in staticUniforms
	 * @see staticUniforms
	 */
	std::vector<UniformTextureDetail> textures;
	/**
	 * Attempts to locate the specified uniform's location and type within the provided shader
	 * @param uniformName The name of the uniform
	 * @param shaderProgram The programId of the shader
	 * @return A pair object whereby the first item is the uniform'd location, and the second item is the type. On failure the first item will be -1
	 * @note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
	 */
	static std::pair<int, GLenum> findUniform(const char *uniformName, const int shaderProgram);
	/**
	 * Attempts to locate the specified attribute's location and type
	 * @param attributeName The name of the attribute
	 * @param shaderProgram The programId of the shaderprogram
	 * @return A pair object whereby the first item is the attribute's location, and the second item is the type. On failure the first item will be -1
	 * @note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
	 */
	static std::pair<int, GLenum> findAttribute(const char *attributeName, const int shaderProgram);
	/**
	 * Subclasses should use this to apply any subclass specific bindings (e.g. vertex attributes)
	 * Called by reload()
	 */
	virtual void _reload() = 0;
	/**
	* Subclasses should use this to clear any enabled client states or attribute arrays
	* Called by clearProgram()
	*/
	virtual void _clearProgram() = 0;
	/**
	* Subclasses should use this to apply any subclass specific shader bindings
	* Called by useProgram()
	*/
	virtual void _useProgram() = 0;


	/**
	 * Loads the text from the provided filepath
	 * @return A pointer to the loaded shader source
	 * @note the returned pointer is allocated via malloc, and should be free'd when nolonger required
	 */
	//static char *loadShaderSource(const char *file);

	/**
	* Looks for the '#version xx' tag in the provided shader source and returns the numeric value
	* @param shaderSource The shader code to detect the version from
	* @return The detected shader version, 0 if one was not found
	*/
	//static unsigned int findShaderVersion(const char *shaderSource);

	/**
	 * Checks whether the specified shader compiled succesfully.
	 * Compilation errors are printed to stderr and compileSuccessflag is set to false on failure.
	 * @param shaderId Location of the shader to check
	 * @param shaderPath Path to the shader being checked (so that it can be easily identified in the error log)
	 * @return True if no errors were detected
	 */
	//bool checkShaderCompileError(const int shaderId, const char *shaderPath);

	/**
	 * Checks whether the specified shader program linked succesfully.
	 * Linking errors are printed to stderr and compileSuccessflag is set to false on failure.
	 * @param programId Location of the shader program to check
	 * @return True if no errors were detected
	 * @note For some reason program compilation failure logs don't seem to work (the same as shader compilation)
	 */
	//bool checkProgramCompileError(const int programId);

	/**
	 * Loads and compiles a shader
	 * @param shaderPath The path to the shader to be loaded
	 * @param shaderType The type of shader: GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
	 * @return The shader program id, -1 if failure
	 * @note This function will also update the value of compileSuccessFlag if failure occurs
	 */
	//int createShader(const char *shaderPath, GLenum shaderType);
};

#endif