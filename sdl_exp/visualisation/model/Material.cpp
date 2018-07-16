#include "Material.h"
#include "../Texture/Texture2D.h"
Material *Material::active = nullptr;
const char * Material::TEX_NAME[12] = { "t_albedo", "t_roughness", "t_metallic", "t_metallic", "t_normal", "t_lightmap", "t_emissive", "t_diffuse", "t_specular", "t_glossiness", "t_specular", "t_glossiness" };
Material::Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const char* name, const bool &shaderRequiresBones)
    : name(name==nullptr?"":name)
    , properties()
    , buffer(buffer)
    , bufferIndex(bufferIndex)
	, hasBaked(false)
	, isWireframe(false)
	, faceCull(true)
	, shaderRequiresBones(shaderRequiresBones)
{
}

Material::Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const Stock::Materials::Material &set, const bool &shaderRequiresBones)
	: Material(buffer, bufferIndex, set.name, shaderRequiresBones)
{
    properties.color = set.color;
    properties.roughness = set.roughness;
    properties.metallic = set.metallic;
    properties.refractionIndex = set.refractionIndex;
    properties.alphaCutOff = set.alphaCutOff;
    properties.emissive = set.emissive;
    properties.diffuse = set.diffuse;
	properties.specular = set.specular;
    properties.glossiness = set.glossiness;
	bake();
}
Material::Material(const Material&other)
	: name(other.name)
	, properties(other.properties)
	, buffer(other.buffer)
	, bufferIndex(other.bufferIndex)
	, hasBaked(false)
	, isWireframe(other.isWireframe)
	, faceCull(other.faceCull)
	, shaderRequiresBones(other.shaderRequiresBones)
{
	for (const auto &i : other.shaders)
		shaders.push_back(std::make_shared<Shaders>(*i));
	if (other.hasBaked)
		bake();
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
/**
 * Todo: compare textures
 */
bool  Material::operator==(Material& other) const
{
    if (
        this->properties.color == other.getColor() &&
        this->properties.roughness == other.getRoughness() &&
        this->properties.metallic == other.getMetallic() &&
        this->properties.refractionIndex == other.getRefractionIndex() &&
        this->properties.alphaCutOff == other.getAlphaCutOff() &&
        this->properties.emissive == other.getEmissive() &&
        this->properties.bitmask == other.properties.bitmask &&
        this->properties.diffuse == other.getDiffuse() &&
        this->properties.specular == other.getSpecular() &&
        this->properties.glossiness == other.getGlossiness() &&
        this->isWireframe == other.getWireframe() &&
        this->faceCull == other.faceCull &&
        this->shaderRequiresBones == other.shaderRequiresBones
        )
    {
        //Compare that they have same texture types (despite bit mask agreement)
        //Also compare texture names match
        for (auto &&t : this->textures)
        {
            if (other.textures.find(t.first) != other.textures.end())
            {
                if (t.second[0].texture->getName() == other.textures[t.first][0].texture->getName())
                    continue;
            }
            return false;            
        }
        return true;
    }
    return false;
}
/**
 * Temporary texture solution before proper materials
 */
void Material::addTexture(TextureFrame texFrame, TextureType type)
{
#ifdef _DEBUG
	//if (type==EnvironmentMap)
	//{
	//	assert(std::dynamic_pointer_cast<const TextureCubeMap>(texFrame.texture));
	//}
#endif
	textures[type].push_back(texFrame);
	this->properties.bitmask |= (1 << type);
	updatePropertiesUniform();
	if (hasBaked)
	{
		if (textures[type].size() == 1)
		{
			defaultShader->addTexture(TEX_NAME[type], texFrame.texture);
			for (const auto &i : this->shaders)
				i->addTexture(TEX_NAME[type], texFrame.texture);
		}
	}
    //This won't currently warn if model has Metallic and MetallicRoughness tex's, inside shader these use same sampler
	if (textures[type].size() > 1)
	{
		fprintf(stderr, "Warning: Material '%s' contains multiple textures of type %s\n Texture stacks are currently unsupported.\n", name.c_str(), TEX_NAME[type]);
	}
}
void Material::setEnvironmentMap(std::shared_ptr<const TextureCubeMap> cubeMap)
{
	////Purge existing environment map
	//textures[EnvironmentMap].clear();
	//if (cubeMap)
	//{
	//	//Create a texture frame
	//	TextureFrame tf;
	//	tf.texture = cubeMap;
	//	//Add it like a regular texture
	//	addTexture(tf, EnvironmentMap);
	//}
}
//HasMatrices overrides
void Material::setViewMatPtr(const glm::mat4 *viewMat)
{
	defaultShader->setViewMatPtr(viewMat);
	for (const auto &i : this->shaders)
		i->setViewMatPtr(viewMat);
}
void Material::setProjectionMatPtr(const glm::mat4 *projectionMat)
{
	defaultShader->setProjectionMatPtr(projectionMat);
	for (const auto &i : this->shaders)
		i->setProjectionMatPtr(projectionMat);
}
void Material::setLightsBuffer(GLuint bufferBindingPoint)
{
	defaultShader->setLightsBuffer(bufferBindingPoint);
	for (const auto &i : this->shaders)
		i->setLightsBuffer(bufferBindingPoint);
}

void Material::setCustomShaders(const std::vector<std::shared_ptr<Shaders>> &shaders)
{
	this->shaders.clear();
	//Clone all shaders
	for (const auto &i:shaders)
		this->shaders.push_back(std::make_shared<Shaders>(*i));
	//Setup material in all Shaders
	for (const auto &i : this->shaders)
	{
		//Setup material buffer
		i->setMaterialBuffer(buffer);
		i->setMaterialID(bufferIndex);
		//Setup default shader, e.g. textures
		for (auto &typeVec : textures)
			if (typeVec.second.size())
				i->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
	}
}
void Material::prepare(unsigned int index)
{
	if (index<shaders.size())
		shaders[index]->prepare();
	else
		defaultShader->prepare();
}
void Material::use(glm::mat4 &transform, unsigned int index, bool requiresPrepare)
{
	if (index<shaders.size())
	{
		shaders[index]->useProgram(requiresPrepare);
		shaders[index]->overrideModelMat(&transform);
	}
	else
	{
		defaultShader->useProgram(requiresPrepare);
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
		//Treat all materials as having alpha until we can add a suitable check/switch
		//if (this->properties.color.a<1.0f)
		//{
		if ((index<shaders.size() && shaders[index]->supportsGL_BLEND())
			|| (index >= shaders.size() && defaultShader->supportsGL_BLEND()))
		{
			GL_CALL(glEnable(GL_BLEND));
		}
		else
		{
			GL_CALL(glDisable(GL_BLEND));
		}
        
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
        //Polygon mode
        GLint polygonMode[2];
        GL_CALL(glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]));
        if ((polygonMode[0] != GL_LINE&&isWireframe) || (polygonMode[0] != GL_FILL&&!isWireframe))
           fprintf(stderr, "Warning: Material::use() detected GL_POLYGON_MODE has been changed.\nUse Material::clearActive() before to prevent this warning.\n");
    }
#endif
}
void Material::clear(unsigned int index)
{
	if (index<shaders.size())
		shaders[index]->clearProgram();
	else
		defaultShader->clearProgram();
}

void Material::reload()
{
	defaultShader->reload();
	for (auto &i : shaders)
		i->reload();
}
void Material::bake()
{
	//Update material properties buffer
	updatePropertiesUniform(true);
	//Create default shader
	if (shaderRequiresBones)
		defaultShader = std::make_shared<Shaders>(Stock::Shaders::BONE);//Temp
	else
        defaultShader = std::make_shared<Shaders>(Stock::Shaders::PBR_TEST);//Temp
    //defaultShader = std::make_shared<Shaders>(Stock::Shaders::PHONG);//Temp
	//Setup material buffer
	defaultShader->setMaterialBuffer(buffer);
	defaultShader->setMaterialID(bufferIndex);
	//Setup default shader, e.g. textures
	for (auto &typeVec:textures)
		if (typeVec.second.size())
			defaultShader->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
	//Setup material in all custom shaders
	for (const auto &i : this->shaders)
	{
		//Setup material buffer
		i->setMaterialBuffer(buffer);
		i->setMaterialID(bufferIndex);
		//Setup default shader, e.g. textures
		for (auto &typeVec : textures)
			if (typeVec.second.size())
				i->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
	}
}