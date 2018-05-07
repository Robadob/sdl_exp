#include "Material.h"
#include "../Texture/Texture2D.h"
Material *Material::active = nullptr;
Material::Material(std::shared_ptr<UniformBuffer> buffer, unsigned int bufferIndex, const char* name)
    : name(name==nullptr?"":name)
    , properties()
    , buffer(buffer)
    , bufferIndex(bufferIndex)
	, hasBaked(false)
	, isWireframe(false)
	, faceCull(true)
	, shaderMode(Phong)
	//, alphaBlendMode{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }//C++11 required (maybe usable VS2015?)
{
	alphaBlendMode[0] = GL_SRC_ALPHA;
	alphaBlendMode[1] = GL_ONE_MINUS_SRC_ALPHA;
	shaders = {  };
}

Material::~Material()
{

}
void Material::updatePropertiesUniform(bool force)
{
	//To save updating the Material buffer during it's initial construction, wait for it to be baked before allowing accessors to update
	if (!force&&!hasBaked)
		return;
	hasBaked = true;
	buffer->setData(&properties, sizeof(MaterialProperties), bufferIndex*sizeof(MaterialProperties));
}
std::shared_ptr<Shaders> Material::getShaders(unsigned int shaderIndex) const
{
	shaderIndex--;
	if (shaderIndex < shaders.size())
		return shaders[shaderIndex];
	return defaultShader;
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
void Material::addTexture(TextureFrame texFrame, TextureType type)
{
	textures[type].push_back(texFrame);
	if (hasBaked)
	{
		defaultShader->addTexture("_texture", texFrame.texture);
		for (auto &&it : shaders)
			if (it && type == (Diffuse && it))
				it->addTexture("_texture", texFrame.texture);
	}
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
	shaderIndex--;
	if (shaderIndex < shaders.size())
	{
		shaders[shaderIndex]->useProgram(false);
		shaders[shaderIndex]->overrideModelMat(&transform);
	}
	else
	{
		defaultShader->useProgram(false);
		defaultShader->overrideModelMat(&transform);
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
			shaders[shaderIndex]->overrideMaterial(bufferIndex);
		else
			defaultShader->overrideMaterial(bufferIndex);
        
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

void Material::bake()
{
	//Update material properties buffer
	updatePropertiesUniform(true);
	//Create default shader
	defaultShader = std::make_shared<Shaders>(Stock::Shaders::TEXTURE_BONE);//Temp
	//Setup default shader, e.g. textures
	if (textures[TextureType::Diffuse].size())
		defaultShader->addTexture("_texture", textures[TextureType::Diffuse][0].texture);//Temp
}