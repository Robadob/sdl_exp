#ifndef __Material_h__
#define __Material_h__
#include "../Texture/Texture.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include "../shader/ShaderHeader.h"
#include "../shader/buffer/UniformBuffer.h"

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
    enum ShadingMode
    {
        Constant,           //No shading, Constant light=1
        Flat,               //Shading on a per face basis, diffuse only
        Gouraud,            //
        Phong,              //
        BlinnPhong,         //
        Toon,               //Multi pass technique where edges are thick black lines and colors are often solid
        OrenNayar,          //Extends Lambertian (diffuse) reflectance to include roughness
        Minnaert,           //Extends Lambertian (diffuse) reflectance to include darkness
        CookTorrance,       //Metallic surface shader
        Fresnel             //The effect of light passing between materials
    };
	static const char * TEX_NAME[13];
    enum TextureType
    {
        None = 0,               //Material properties not related to textures?
		Ambient = 1,            //Combined with ambient light value
        Diffuse = 2,            //Combined with diffuse light value
        Specular = 3,           //Combined with specular light value
        Emmisive = 4,           //Added to the result of the light equation
        Height = 5,             //Height map
        Normals = 6,            //Normal map
        Shininess = 7,          //Glossiness of the material
        Opacity = 8,            //Opacity
        Displacement = 9,       //Displacement map (none-standard handling required)
        LightMap = 10,          //Ambient occlusion, how bright light is at the specified point
        Reflection = 11,        //Perfect mirror reflection (slow to calculate)
        Unknown =12             //Unknown, will be treated as diffuse if diffuse isn't present
    };
    /**
     * Holds a Texture and it's relevant blending operations
     */
    struct TextureFrame
    {
        enum BlendOperation
        {
            Multiply,       //T1 * T2
            Add,            //T1 + T2
            Subtract,       //T1 - T2
            Divide,         //T1 / T2
            SmoothAdd,      //(T1 + T2) - (T1 * T2)
            SignedAdd,       //T1 + (T2-0.5)
			Null
        };
        std::shared_ptr<const Texture> texture;
        float weight=1.0f;//This value is multiplied by the textures color
        unsigned int uvIndex=0;//Some meshes have dual texture coordinates
        BlendOperation operation;//How to process the n-1 texture?
        GLenum mappingModeU;//GL_TEXTURE_WRAP_S=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        GLenum mappingModeV;//GL_TEXTURE_WRAP_T=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        bool isDecal=false;//Invalid tex coords cause frame dump
        bool invertColor = false;//color=1-color
        bool useAlpha = true;//useAlpha if present
    };
    /**
     * Creates a named material with the default configuration
     */
	Material(std::shared_ptr<UniformBuffer> buffer, unsigned int bufferIndex, const char* name="");
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
     * Enables the materials settings
     */
	void use(glm::mat4 &transform, const std::shared_ptr<Shaders> &shaders = nullptr);
    static void clearActive() { Material::active = nullptr; }
    //Setters
    void setName(const std::string name) { this->name = name; };

    void setDiffuse(const glm::vec3 diffuse) { this->properties.diffuse = diffuse; updatePropertiesUniform(); }
    void setSpecular(const glm::vec3 specular) { this->properties.specular = specular; updatePropertiesUniform(); }
    void setAmbient(const glm::vec3 ambient) { this->properties.ambient = ambient; updatePropertiesUniform(); }
    void setEmissive(const glm::vec3 emissive) { this->properties.emissive = emissive; updatePropertiesUniform(); }
    void setTransparent(const glm::vec3 transparent) { this->properties.transparent = transparent; updatePropertiesUniform(); }
    void setOpacity(const float opacity) { this->properties.opacity = opacity; updatePropertiesUniform(); }
    void setShininess(const float shininess) { this->properties.shininess = shininess; updatePropertiesUniform(); }
    void setShininessStrength(const float shininessStrength) { this->properties.shininessStrength = shininessStrength; updatePropertiesUniform(); }
    void setRefractionIndex(const float refractionIndex) { this->properties.refractionIndex = refractionIndex; updatePropertiesUniform(); }
    
    void setWireframe(const bool isWireframe) { this->isWireframe = isWireframe; }
	void setTwoSided(const bool twoSided) { this->faceCull = !twoSided; }
    void setShadingMode(const ShadingMode shaderMode) { this->shaderMode = shaderMode; }
	void addTexture(TextureFrame, TextureType type = Diffuse);

    /**
     * @see glBlendFunc()
     */
    void setAlphaBlendMode(const GLenum sfactor, const GLenum dfactor) { this->alphaBlendMode[0] = sfactor; this->alphaBlendMode[1] = dfactor; }

    //Getters
    std::string getName() const { return name; }
    glm::vec3 getDiffuse() const { return properties.diffuse; }
    glm::vec3 getSpecular() const { return properties.specular; }
    glm::vec3 getAmbient() const { return properties.ambient; }
    glm::vec3 getEmissive() const { return properties.emissive; }
    glm::vec3 getTransparent() const { return properties.transparent; }
    float getOpacity() const { return properties.opacity; }
    float getShininess() const { return properties.shininess; }
    float getShininessStrength() const { return properties.shininessStrength; }
    float getRefractionIndex() const { return properties.refractionIndex; }
    bool getWireframe() const { return isWireframe; }
    bool getTwoSided() const { return !faceCull; }
    ShadingMode getShadingMode() const { return shaderMode; }
	std::shared_ptr<Shaders> getShaders() const { return defaultShader; }
    std::pair<GLenum, GLenum> getAlphaBlendMode() const { return std::make_pair(alphaBlendMode[0], alphaBlendMode[1]); }
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
	ShadingMode shaderMode;
	std::shared_ptr<Shaders> defaultShader;
    GLenum alphaBlendMode[2]; //GL_SRC_ALPHA, GL_ONE for additive blending//GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA for default
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