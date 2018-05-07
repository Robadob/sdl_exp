#include "Material.h"
#include "../Texture/Texture2D.h"
Material *Material::active = nullptr;
Material::Material(const char* name)
    : name(name==nullptr?"":name)
    , properties()
    , propertiesUniformBuffer(0)
    , isWireframe(false)
    , faceCull(true)
	, shaderMode(Phong)
	//, alphaBlendMode{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }//C++11 required (maybe usable VS2015?)
{
	alphaBlendMode[0] = GL_SRC_ALPHA;
	alphaBlendMode[1] = GL_ONE_MINUS_SRC_ALPHA;
    GL_CALL(glGenBuffers(1, &propertiesUniformBuffer));
    updatePropertiesUniform();
	shaders = { std::make_shared<Shaders>(Stock::Shaders::TEXTURE_BONE) };
}

Material::~Material()
{
    GL_CALL(glDeleteBuffers(1, &propertiesUniformBuffer));
}
void Material::updatePropertiesUniform()
{
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, propertiesUniformBuffer));
    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialProperties), &properties, GL_STATIC_READ));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
std::shared_ptr<Shaders> Material::getShaders(unsigned int shaderIndex) const
{
	if (shaderIndex < shaders.size())
		return shaders[shaderIndex];
	return std::shared_ptr<Shaders>(0);
}
/**
 * Todo: compare textures
 */
bool  Material::operator==(Material& other) const
{
    if (
        this->properties.diffuse == other.getDiffuse() &&
        this->properties.specular == other.getSpecular() &&
        this->properties.ambient == other.getAmbient() &&
        this->properties.emissive == other.getEmissive() &&
        this->properties.transparent == other.getTransparent() &&
        this->properties.opacity == other.getOpacity() &&
        this->properties.shininess == other.getShininess() &&
        this->properties.shininessStrength == other.getShininessStrength() &&
        this->properties.refractionIndex == other.getRefractionIndex() &&
        this->isWireframe == other.getWireframe() &&
        this->faceCull == !other.getTwoSided() &&
        this->shaderMode == other.getShadingMode() &&
        this->getAlphaBlendMode() == other.getAlphaBlendMode()
        )
        return true;
    return false;
}
/**
 * Temporary texture solution before proper materials
 */
void Material::addTexture(std::shared_ptr<const Texture> texPtr, TextureType type)
{
	if (!texPtr)
	{
		fprintf(stderr, "Material::addTexture() Tex missing\n");
		return;
	}
	TextureFrame t;
	t.texture = texPtr;
	textures[type].push_back(t);
	for (auto &&it : shaders)
		if (it && type == (Diffuse && it))
			it->addTexture("_texture", texPtr);	
}
//HasMatrices overrides
void Material::setViewMatPtr(const glm::mat4 *viewMat)
{
	for (auto &&it : shaders)
		if (it)
			it->setViewMatPtr(viewMat);
}
void Material::setProjectionMatPtr(const glm::mat4 *projectionMat)
{
	for (auto &&it : shaders)
		if (it)
			it->setProjectionMatPtr(projectionMat);
}

void Material::use(glm::mat4 &transform, unsigned int shaderIndex)
{
	if (shaderIndex < shaders.size())
	{
		shaders[shaderIndex]->useProgram(false);
		shaders[shaderIndex]->overrideModelMat(&transform);
	}
    if (active != this)
	{
        //Setup GL states for material
        if (this->isWireframe)
        {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        else
        {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (this->faceCull)
		{
			GL_CALL(glEnable(GL_CULL_FACE));
        }
        else
		{
			GL_CALL(glDisable(GL_CULL_FACE));
        }
        
        GL_CALL(glBlendFunc(alphaBlendMode[0], alphaBlendMode[1]));

        //Setup material properties with shader (If we can guarantee shader is unique, we can skip this)
		if (shaderIndex < shaders.size())
			shaders[shaderIndex]->overrideMaterial(propertiesUniformBuffer);
        
    	active = this;
    }
#ifdef _DEBUG
    else
    {
        //Cull face
		GLboolean cullFace;
        GL_CALL(glGetBooleanv(GL_CULL_FACE, &cullFace));//0 if face culling disabled
        if ((cullFace!=0) != faceCull)
		{
			fprintf(stderr, "Warning: Material::use() detected GL_CULL_FACE has been changed.\nUse Material::clearActive() before to prevent this warning.\n");
        }
        ////Polygon mode
        GLint polygonMode[2];
        GL_CALL(glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]));
        if ((polygonMode[0] != GL_LINE&&isWireframe) || (polygonMode[0] != GL_FILL&&!isWireframe))
           fprintf(stderr, "Warning: Material::use() detected GL_POLYGON_MODE has been changed.\nUse Material::clearActive() before to prevent this warning.\n");
        //Blend mode
        GLint alphaSrc, alphaDst, rgbSrc, rgbDst;
        GL_CALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, &alphaSrc));
        GL_CALL(glGetIntegerv(GL_BLEND_DST_ALPHA, &alphaDst));
        GL_CALL(glGetIntegerv(GL_BLEND_SRC_RGB, &rgbSrc));
        GL_CALL(glGetIntegerv(GL_BLEND_DST_RGB, &rgbDst));
        if (alphaSrc != alphaBlendMode[0] || rgbSrc != alphaBlendMode[0] || alphaDst != alphaBlendMode[1] || rgbDst != alphaBlendMode[1])
        {
            fprintf(stderr, "Warning: Material::use() detected glBlendFunc() has been called.\nUse Material::clearActive() before to prevent this warning.\n");
        }

    }
#endif
}

void Material::reload()
{
#pragma message ( "TODO : Material::reload()" )
}

void setViewMatPtr(const Camera* camera)
{
#pragma message ( "TODO : Material::setViewMatPtr()" )
}