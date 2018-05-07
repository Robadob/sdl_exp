#ifndef __Material_h__
#define __Material_h__
#include "../Texture/Texture.h"
#include "../interface/Renderable.h"
#include "../shader/Shaders.h"
#include "../shader/ShaderHeader.h"
class Material : protected Renderable
{
    /**
     * Pointer to the last used material
     * Used to prevent rebinding material states if unchanged
     */
    static Material *active;
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
    enum TextureType
    {
        None,               //Material properties not related to textures?
        Diffuse,            //Combined with diffuse light value
        Specular,           //Combined with specular light value
        Ambient,            //Combined with ambient light value
        Emmisive,           //Added to the result of the light equation
        Height,             //Height map
        Normals,            //Normal map
        Shininess,          //Glossiness of the material
        Opacity,            //Opacity
        Displacement,       //Displacement map (none-standard handling required)
        LightMap,           //Ambient occlusion, how bright light is at the specified point
        Reflection,         //Perfect mirror reflection (slow to calculate)
        Unknown             //Unknown, will be treated as diffuse if diffuse isn't present
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
            SignedAdd       //T1 + (T2-0.5)
        };
        std::shared_ptr<const Texture> texture;
        float weight=1.0f;//This value is multiplied by the textures color
        float uvIndex=0;//Some meshes have dual texture coordinates
        BlendOperation operation;//How to process the n-1 texture?
        //MAPPING: aiProcess_GenUVCoords() to convert coords 
        GLenum mappingModeU;//GL_TEXTURE_WRAP_S=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        GLenum mappingModeV;//GL_TEXTURE_WRAP_T=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        bool isDecal=false;//Invalid tex coords cause frame dump
        bool invertColor = false;//color=1-color
        bool useAlpha = true;//useAlpha if present
    };
public:
    /**
     * Creates a named material with the default configuration
     */
    Material(const char* name = nullptr);
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
	void use(glm::mat4 &transform, unsigned int shaderIndex = 0);
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
#pragma message ("TODO: remove Material::setShader()")
    //void setShader(const std::shared_ptr<Shaders> shader)
    //{
    //    //We should make a copy of this shader
    //    this->shaders = shader;
    //};
	void addTexture(std::shared_ptr<const Texture> texPtr, TextureType type = Diffuse);

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
	std::shared_ptr<Shaders> getShaders(unsigned int shaderIndex = 0) const;
	size_t getShadersCount() const { return shaders.size(); }
    std::pair<GLenum, GLenum> getAlphaBlendMode() const { return std::make_pair(alphaBlendMode[0], alphaBlendMode[1]); };
private:
    //Id
    std::string name;

    MaterialProperties properties;
    GLuint propertiesUniformBuffer;
    void updatePropertiesUniform();
    //Textures
    std::map<TextureType, std::vector<TextureFrame>> textures;
    //Modifiers
    bool isWireframe;
    bool faceCull;
	ShadingMode shaderMode;
	std::vector<std::shared_ptr<Shaders>> shaders;
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
    * Sets the pointer from which the Model matrix should be loaded from
    * @param modelMat A pointer to the modelMatrix to be tracked
    * @note This pointer is likely provided by the Visualisation object
    */
    void setModelMatPtr(const glm::mat4 *modelMat) override;
    /**
    * Overrides the model matrix (and all dependent matrices) until useProgram() is next called
    * @param modelMat Pointer to the overriding modelMat
    */
    void overrideModelMat(const glm::mat4 *modelMat) override;
	void reload() override;
};
#endif //__Material_h__