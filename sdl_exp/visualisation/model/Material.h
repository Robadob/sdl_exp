#ifndef __Material_h__
#define __Material_h__

#include "../Texture/Texture.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include "../shader/ShaderHeader.h"
#include "../shader/buffer/UniformBuffer.h"
#include "../texture/TextureCubeMap.h"

namespace Stock
{
	namespace Materials
	{
		struct Material
		{
            Material(const char *name, const glm::vec4 &color, const float &roughness, const float &metallic, const glm::vec4 &diffuse = glm::vec4(0), const glm::vec3 &specular = glm::vec3(1.0f), const float &glossiness = 0.0f)
				: name(name)
                , color(color)
                , roughness(roughness)
                , metallic(metallic)
                , refractionIndex(1.05f)//Air
                , alphaCutOff(0)
                , emissive(0)
                , diffuse(diffuse)
                , specular(specular)
                , glossiness(glossiness)
			{}
			const char *name;
            const glm::vec4 color;          //Base color (aka diffuse, albedo)
            const float roughness;
            const float metallic;
            const float refractionIndex;	//Unused (padding)
            const float alphaCutOff;
            const glm::vec3 emissive;       //Unused, Emissive color (light emitted)
            //Extension: KHR_materials_pbrSpecularGlossiness
            const glm::vec4 diffuse;        //Diffuse reflection color
            const glm::vec3 specular;       //Specular reflection color
            const float glossiness;         //Glossiness factor
		};
	//	const Material WHITE{ "white", glm::vec3(0.1f), glm::vec3(1), glm::vec3(1), 76.8f, 1.0f };
	//	const Material VANTABLACK{ "null", glm::vec3(0), glm::vec3(0), glm::vec3(0), 0.0f, 1.0f };
	//	const Material EMERALD{ "emerald", glm::vec3(0.0215f, 0.1745f, 0.0215f), glm::vec3(0.07568f, 0.61424f, 0.07568f), glm::vec3(0.633f, 0.727811f, 0.633f), 76.8f, 1.0f };
	//	const Material JADE{ "jade", glm::vec3(0.135f, 0.2225f, 0.1575f), glm::vec3(0.54f, 0.89f, 0.63f), glm::vec3(0.316228f), 12.8f, 1.0f };
	//	const Material OBSIDIAN{ "obsidian", glm::vec3(0.05375f, 0.05f, 0.06625f), glm::vec3(0.18275f, 0.17f, 0.22525f), glm::vec3(0.332741f, 0.328634f, 0.346435f), 38.4f, 1.0f };
	//	const Material PEARL{ "pearl", glm::vec3(0.25f, 0.20725f, 0.20725f), glm::vec3(1.0f, 0.829f, 0.829f), glm::vec3(0.296648f), 11.264f, 1.0f };
	//	const Material RUBY{ "ruby", glm::vec3(0.1745f, 0.01175f, 0.01175f), glm::vec3(0.61424f, 0.04136f, 0.04136f), glm::vec3(0.727811f, 0.626959f, 0.626959f), 76.8f, 1.0f };
	//	const Material TURQUOISE{ "turquoise", glm::vec3(0.1f, 0.18725f, 0.1745f), glm::vec3(0.396f, 0.74151f, 0.69102f), glm::vec3(0.297254f, 0.30829f, 0.306678f), 12.8f, 1.0f };
	//	const Material BRASS{ "brass", glm::vec3(0.329412f, 0.223529f, 0.027451f), glm::vec3(0.780392f, 0.568627f, 0.113725f), glm::vec3(0.992157f, 0.941176f, 0.807843f), 27.89f, 1.0f };
	//	const Material BRONZE{ "bronze", glm::vec3(0.2125f, 0.1275f, 0.054f), glm::vec3(0.714f, 0.4284f, 0.18144f), glm::vec3(0.393548f, 0.271906f, 0.166721f), 25.6f, 1.0f };
	//	const Material CHROME{ "chrome", glm::vec3(0.25f), glm::vec3(0.4f), glm::vec3(0.774597f), 0.6f, 1.0f };
	//	const Material COPPER{ "copper", glm::vec3(0.19125f, 0.0735f, 0.0225f), glm::vec3(0.7038f, 0.27048f, 0.0828f), glm::vec3(0.256777f, 0.137622f, 0.086014f), 12.8f, 1.0f };
	//	const Material GOLD{ "gold", glm::vec3(0.24725f, 0.1995f, 0.0745f), glm::vec3(0.75164f, 0.60648f, 0.22648f), glm::vec3(0.628281f, 0.555802f, 0.366065f), 51.2f, 1.0f };
	//	const Material SILVER{ "silver", glm::vec3(0.19225f), glm::vec3(0.50754f), glm::vec3(0.508273f), 51.2f, 1.0f };
	//	const Material BLACK_PLASTIC{ "black plastic", glm::vec3(0), glm::vec3(0.01f), glm::vec3(0.5f), 32.0f, 1.0f };
	//	const Material CYAN_PLASTIC{ "cyan plastic", glm::vec3(0.0f, 0.1f, 0.06f), glm::vec3(0.0f, 0.50980392f, 0.50980392f), glm::vec3(0.50196078f), 32.0f, 1.0f };
	//	const Material GREEN_PLASTIC{ "green plastic", glm::vec3(0), glm::vec3(0.1f, 0.35f, 0.1f), glm::vec3(0.45, 0.55f, 0.45f), 32.0f, 1.0f };
	//	const Material RED_PLASTIC{ "red plastic", glm::vec3(0), glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.7f, 0.6f, 0.6f), 32.0f, 1.0f };
	//	const Material WHITE_PLASTIC{ "white plastic", glm::vec3(0), glm::vec3(0.55f), glm::vec3(0.7f), 32.0f, 1.0f };
	//	const Material YELLOW_PLASTIC{ "yellow plastic", glm::vec3(0), glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.6f, 0.6f, 0.5f), 10.0f, 1.0f };
	//	const Material BLACK_RUBBER{ "black rubber", glm::vec3(0.02f), glm::vec3(0.01f), glm::vec3(0.4f), 10.0f, 1.0f };
	//	const Material CYAN_RUBBER{ "cyan rubber", glm::vec3(0.0f, 0.05f, 0.05f), glm::vec3(0.4f, 0.5f, 0.5f), glm::vec3(0.04f, 0.7f, 0.7f), 10.0f, 1.0f };
	//	const Material GREEN_RUBBER{ "green rubber", glm::vec3(0.0f, 0.05f, 0.0f), glm::vec3(0.4f, 0.5f, 0.4f), glm::vec3(0.04f, 0.7f, 0.04f), 10.0f, 1.0f };
	//	const Material RED_RUBBER{ "red rubber", glm::vec3(0.05f, 0.0f, 0.0f), glm::vec3(0.5f, 0.4f, 0.4f), glm::vec3(0.7f, 0.04f, 0.04f), 10.0f, 1.0f };
	//	const Material WHITE_RUBBER{ "white rubber", glm::vec3(0.05f), glm::vec3(0.5f), glm::vec3(0.7f), 10.0f, 1.0f };
	//	const Material YELLOW_RUBBER{ "yellow rubber", glm::vec3(0.05f, 0.05f, 0.0f), glm::vec3(0.5f, 0.5f, 0.4f), glm::vec3(0.7f, 0.7f, 0.04f), 10.0f, 1.0f };
	}
	namespace RefractionIndex
	{//https://en.wikipedia.org/wiki/List_of_refractive_indices
		const float AIR		= 1.0f;
		const float WATER	= 1.33f;
		const float ICE		= 1.31f;
		const float GLASS	= 1.52f;
		const float DIAMOND = 2.417f;
		const float SAPPHIRE = 1.77f;
		const float CUBIC_ZIRCONIA = 2.165f;
	}
}

/**
 * This class holds all the necessary data to configure a material
 * It holds a default shader specific to the material, however can be passed a shared shader with useMaterial()
 */
class Material : protected Renderable
{
    /**
     * Pointer to the last used material
     * Used to prevent rebinding material states if unchanged
     * Less than ideal this being static, should be hosted in model data
     */
    static Material *active;
public:
	static const char *TEX_NAME[14];
    enum TextureType
    {
        Albedo = 0,
		Roughness = 1,
        Metallic = 2,
        MetallicRoughness = 3,
        Normal = 4,
        LightMap = 5, //Ambient Occlusion
        Emissive = 6,
        //Extension: KHR_materials_pbrSpecularGlossiness
        Diffuse = 7,
        Specular = 8,
        Glossiness = 9,
        SpecularGlossiness = 10,
        Shininess = 11,  //1.0-roughness
    };
    /**
     * Holds a Texture and it's relevant blending operations
     */
    struct TextureFrame
    {
        std::shared_ptr<const Texture> texture = nullptr;
		GLenum mappingModeU = GL_REPEAT;//GL_TEXTURE_WRAP_S=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
		GLenum mappingModeV = GL_REPEAT;//GL_TEXTURE_WRAP_T=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
    };
    /**
     * Creates a named material with the default configuration
     */
	Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const char* name = "", const bool &shaderRequiresBones = false);
	/**
	* Creates a named material with the default configuration
	*/
	Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const Stock::Materials::Material &set, const bool &shaderRequiresBones = false);
	Material(const Material&other);
    /**
     *
     */
    virtual ~Material();
    /**
     * @return True if the internal configuration of the materials is identical
     * @note The material name and shaders are not considered a configuration
     */
    bool operator==(Material& other) const;
	/**
	 * Prepares the indexed shader
	 * @param index The index of the chosen shader, out of bound uses default for the material
	 */
	void prepare(unsigned int index = UINT_MAX);
    /**
     * Enables the materials settings and selects the corresponding shader
     * @param transform The model matrix to be used
     * @param index The shader index to use, leave as default if default shader is to be used
     * @param requiresPrepare Set to true when the material's shader has not be prepared at the start of the rendercall. (This should be done if a material may be used multiple times alongside others whilst rendering a model)
     */
	void use(glm::mat4 &transform, unsigned int index = UINT_MAX, bool requiresPrepare = true);
	/**
	 * Clears the indexed shader
	 * @param index The index of the chosen shader, out of bound uses default for the material
	 */
	void clear(unsigned int index = UINT_MAX);
    static void clearActive() { Material::active = nullptr; }
    //Setters
    void setName(const std::string name) { this->name = name; };
    void setColor(const glm::vec4 &color) { this->properties.color = color; updatePropertiesUniform(); }
    void setRoughness(const float &roughness) { this->properties.roughness = roughness; updatePropertiesUniform(); }
    void setMetallic(const float &metallic) { this->properties.metallic = metallic; updatePropertiesUniform(); }
    void setRefractionIndex(const float &refractionIndex) { this->properties.refractionIndex = refractionIndex; updatePropertiesUniform(); }
    void setAlphaCutOff(const float &alphaCutOff) { this->properties.alphaCutOff = alphaCutOff; updatePropertiesUniform(); }
    void setEmissive(const glm::vec3 &emissive) { this->properties.emissive = emissive; updatePropertiesUniform(); }
    void setTwoSided(const bool &twoSided) { this->faceCull = !twoSided; }
    //Extension: KHR_materials_pbrSpecularGlossiness
    void setDiffuse(const glm::vec4 &diffuse) { this->properties.diffuse = diffuse; updatePropertiesUniform(); }
    void setSpecular(const glm::vec3 &specular) { this->properties.specular = specular; updatePropertiesUniform(); }
    void setGlossiness(const float &glossiness) { this->properties.glossiness = glossiness; updatePropertiesUniform(); }
    //Convenience
    void setOpacity(const float &opacity) { this->properties.color.a = opacity; updatePropertiesUniform(); }
    void setShininess(const float &shininess) { this->properties.roughness = 1.0f-shininess; updatePropertiesUniform(); }
    //Render config
    void setWireframe(const bool isWireframe) { this->isWireframe = isWireframe; }
    //Textures
	void addTexture(TextureFrame, TextureType type = Diffuse);
	/**
	 * Sets the environment map used for reflection
	 * Static maps are likely to be the skybox texture
	 */
    void setEnvironmentMap(std::shared_ptr<const TextureCubeMap> cubeMap);
    void setCustomShaders(const std::vector<std::shared_ptr<Shaders>> &shaders);
    
    //Getters
    std::string getName() const { return name; }
    glm::vec4 getColor() const { return properties.color; }
    float getRoughness() const { return properties.roughness; }
    float getMetallic() const { return properties.metallic; }
    float getRefractionIndex() const { return properties.refractionIndex; }
    float getAlphaCutOff() const { return properties.alphaCutOff; }
    glm::vec3 getEmissive() const { return properties.emissive; }
    bool getTwoSided() const { return !faceCull; }
    //Extension: KHR_materials_pbrSpecularGlossiness
    glm::vec4 getDiffuse() const { return properties.diffuse; }
    glm::vec3 getSpecular() const { return properties.specular; }
    float getGlossiness() const { return properties.glossiness; }
    //Convenience
    float getOpacity() const { return properties.color.a; }
    float getShininess() const { return 1.0f - properties.roughness; }
	//Render config
    bool getWireframe() const { return isWireframe; }
	std::shared_ptr<Shaders> getShaders(unsigned int index = UINT_MAX) const { return index<shaders.size() ? shaders[index] : defaultShader; }
	/**
	 * Creates and sets up the default shader
	 * Copies material properties to the uniform buffer for the first time
	 */
	void bake();
private:
    //Id
    std::string name;

    MaterialProperties properties;
	std::shared_ptr<UniformBuffer> buffer;
	const unsigned int bufferIndex;
	bool hasBaked;
    void updatePropertiesUniform(bool force=false);
    //Textures
    std::map<TextureType, std::vector<TextureFrame>> textures;
    //Modifiers
    bool isWireframe;
    bool faceCull;
	std::shared_ptr<Shaders> defaultShader;
	std::vector<std::shared_ptr<Shaders>> shaders;
	bool shaderRequiresBones;
//HasMatrices overrides
public:
    /**
    * Sets the pointer from which the View matrix should be loaded from
    * @param viewMat A pointer to the viewMatrix to be tracked
    * @note This pointer is likely provided by a Camera subclass
    */
    void setViewMatPtr(const glm::mat4  *viewMat) override;
    /**
    * Sets the pointer from which the Projection matrix should be loaded from
    * @param projectionMat A pointer to the projectionMatrix to be tracked
    * @note This pointer is likely provided by the Visualisation object
    */
	void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
	/**
	* Provides lights buffer to the shader
	* @param bufferBindingPoint Set the buffer binding point to be used for rendering
	*/
	void setLightsBuffer(GLuint bufferBindingPoint) override;
	void reload() override;
};
#endif //__Material_h__