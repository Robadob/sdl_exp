#ifndef __Model_assimpUtils_h__
#define __Model_assimpUtils_h__

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../texture/Texture2D.h"

/**
 * These are GLTF material props pulled from glTF2Asset.h
 */
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR "$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR "$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR "$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE aiTextureType_DIFFUSE, 1
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE aiTextureType_UNKNOWN, 0
#define AI_MATKEY_GLTF_ALPHAMODE "$mat.gltf.alphaMode", 0, 0
#define AI_MATKEY_GLTF_ALPHACUTOFF "$mat.gltf.alphaCutoff", 0, 0
#define AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS "$mat.gltf.pbrSpecularGlossiness", 0, 0
#define AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS_GLOSSINESS_FACTOR "$mat.gltf.pbrMetallicRoughness.glossinessFactor", 0, 0

#define _AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE "$tex.file.texCoord"
#define _AI_MATKEY_GLTF_MAPPINGNAME_BASE "$tex.mappingname"
#define _AI_MATKEY_GLTF_MAPPINGID_BASE "$tex.mappingid"
#define _AI_MATKEY_GLTF_MAPPINGFILTER_MAG_BASE "$tex.mappingfiltermag"
#define _AI_MATKEY_GLTF_MAPPINGFILTER_MIN_BASE "$tex.mappingfiltermin"

#define AI_MATKEY_GLTF_TEXTURE_TEXCOORD _AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, type, N
#define AI_MATKEY_GLTF_MAPPINGNAME(type, N) _AI_MATKEY_GLTF_MAPPINGNAME_BASE, type, N
#define AI_MATKEY_GLTF_MAPPINGID(type, N) _AI_MATKEY_GLTF_MAPPINGID_BASE, type, N
#define AI_MATKEY_GLTF_MAPPINGFILTER_MAG(type, N) _AI_MATKEY_GLTF_MAPPINGFILTER_MAG_BASE, type, N
#define AI_MATKEY_GLTF_MAPPINGFILTER_MIN(type, N) _AI_MATKEY_GLTF_MAPPINGFILTER_MIN_BASE, type, N
enum class SamplerMagFilter : unsigned int
{
    UNSET = 0,
    SamplerMagFilter_Nearest = 9728,
    SamplerMagFilter_Linear = 9729
};

//! Values for the Sampler::minFilter field
enum class SamplerMinFilter : unsigned int
{
    UNSET = 0,
    SamplerMinFilter_Nearest = 9728,
    SamplerMinFilter_Linear = 9729,
    SamplerMinFilter_Nearest_Mipmap_Nearest = 9984,
    SamplerMinFilter_Linear_Mipmap_Nearest = 9985,
    SamplerMinFilter_Nearest_Mipmap_Linear = 9986,
    SamplerMinFilter_Linear_Mipmap_Linear = 9987
};
enum class SamplerWrap : unsigned int
{
    UNSET = 0,
    Clamp_To_Edge = 33071,
    Mirrored_Repeat = 33648,
    Repeat = 10497
};
/**
 * Methods are stored here to hide assimp from header files
 * and to provide utilities for converting assimp objects to our local formats
 * without bloating Model.cpp
 * @note http://assimp.sourceforge.net/lib_html/structai_material.html
 * @note http://assimp.sourceforge.net/lib_html/materials.html
 * @note http://assimp.sourceforge.net/lib_html/material_8h.html
 */
namespace au
{
	inline Material::TextureType toTexType_internal(const aiTextureType &t)
    {
        if (t == aiTextureType_UNKNOWN) //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE
            return Material::TextureType::MetallicRoughness;
        if (t == aiTextureType_DIFFUSE) //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE
            return Material::TextureType::Albedo;
        if (t == aiTextureType_NORMALS)
            return Material::TextureType::Normal;
        if (t == aiTextureType_LIGHTMAP)
            return Material::TextureType::LightMap;
        if (t == aiTextureType_EMISSIVE)
            return Material::TextureType::Emissive;
        //if (t == aiTextureType_Metallic)
        //    return Material::TextureType::Metallic;
        //if (t == aiTextureType_Roughness)
        //    return Material::TextureType::Roughness;
        //Extension: KHR_materials_pbrSpecularGlossiness
		if (t == aiTextureType_DIFFUSE)
			return Material::TextureType::Diffuse;
		if (t == aiTextureType_SPECULAR)
			return Material::TextureType::Specular;
        //Convenience
        if (t == aiTextureType_SHININESS)
            return Material::TextureType::Shininess;
		throw std::runtime_error("Unexpected aiTextureType value when parsing aiMaterial.");
	}
    inline unsigned long long toMagFilter(const SamplerMagFilter &mag)
	{
        if (mag == SamplerMagFilter::SamplerMagFilter_Nearest)
            return Texture::FILTER_MAG_NEAREST;
        //if (mag == SamplerMagFilter::SamplerMagFilter_Linear)
            return Texture::FILTER_MAG_LINEAR;
	}
    inline unsigned long long toMinFilter(const SamplerMinFilter &min)
    {
        if (min == SamplerMinFilter::SamplerMinFilter_Nearest)
            return Texture::FILTER_MIN_NEAREST;
        if (min == SamplerMinFilter::SamplerMinFilter_Linear)
            return Texture::FILTER_MIN_LINEAR;
        if (min == SamplerMinFilter::SamplerMinFilter_Nearest_Mipmap_Nearest)
            return Texture::FILTER_MIN_NEAREST_MIPMAP_NEAREST;
        if (min == SamplerMinFilter::SamplerMinFilter_Linear_Mipmap_Nearest)
            return Texture::FILTER_MIN_LINEAR_MIPMAP_NEAREST;
        if (min == SamplerMinFilter::SamplerMinFilter_Nearest_Mipmap_Linear)
            return Texture::FILTER_MIN_NEAREST_MIPMAP_LINEAR;
        //if (min == SamplerMinFilter::SamplerMinFilter_Linear_Mipmap_Linear)
            return Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR;
	}
	inline GLuint toMapMode(aiTextureMapMode &mapMode)
	{
        if (mapMode == aiTextureMapMode_Wrap || (int)mapMode == (int)SamplerWrap::Repeat)
			return GL_REPEAT;
        if (mapMode == aiTextureMapMode_Clamp || (int)mapMode == (int)SamplerWrap::Clamp_To_Edge || (int)mapMode == (int)SamplerWrap::UNSET)
			return GL_CLAMP_TO_EDGE;
		if (mapMode == aiTextureMapMode_Decal)
			return GL_CLAMP_TO_EDGE;//Want border to be transparent here!
        if (mapMode == aiTextureMapMode_Mirror || (int)mapMode == (int)SamplerWrap::Mirrored_Repeat )
			return GL_MIRRORED_REPEAT;
		throw std::runtime_error("Unexpected aiTextureMapMode value when parsing aiMaterial.");
	}
	inline unsigned long long toMapMode_internal(const aiTextureMapMode &mapModeU, const aiTextureMapMode &mapModeV)
	{
		unsigned long long rtn = 0;
		{//u
			if (mapModeU == aiTextureMapMode_Wrap)
				rtn |= Texture::WRAP_REPEAT_U;
			if (mapModeU == aiTextureMapMode_Clamp)
				rtn |= Texture::WRAP_CLAMP_TO_EDGE_U;
			if (mapModeU == aiTextureMapMode_Decal)
				rtn |= Texture::WRAP_CLAMP_TO_BORDER_U;//Want border to be transparent here!
			if (mapModeU == aiTextureMapMode_Mirror)
				rtn |= Texture::WRAP_MIRRORED_REPEAT_U;
		}
		{//v
			if (mapModeV == aiTextureMapMode_Wrap)
				rtn |= Texture::WRAP_REPEAT_V;
			if (mapModeV == aiTextureMapMode_Clamp)
				rtn |= Texture::WRAP_CLAMP_TO_EDGE_V;
			if (mapModeV == aiTextureMapMode_Decal)
				rtn |= Texture::WRAP_CLAMP_TO_BORDER_V;//Want border to be transparent here!
			if (mapModeV == aiTextureMapMode_Mirror)
				rtn |= Texture::WRAP_MIRRORED_REPEAT_V;
		}
		return rtn;
	}
    //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR
    //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR
    //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR
    //AI_MATKEY_SHININESS (roughness inverted to shininess)
    //AI_MATKEY_GLTF_MAPPINGFILTER_MIN
    //AI_MATKEY_GLTF_MAPPINGFILTER_MAG
    //aiTextureType_NORMALS
    //aiTextureType_LIGHTMAP (AO tex)
    //aiTextureType_EMISSIVE
    //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE
    //AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE
    //AI_MATKEY_COLOR_EMISSIVE
    //AI_MATKEY_GLTF_ALPHACUTOFF
    //AI_MATKEY_GLTF_ALPHAMODE
    //AI_MATKEY_TWOSIDED

    //IBL stuff?
    //AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS_GLOSSINESS_FACTOR
    //AI_MATKEY_COLOR_DIFFUSE
    //AI_MATKEY_COLOR_SPECULAR
    //aiTextureType_DIFFUSE
    //aiTextureType_SPECULAR
    inline void getMaterialProps(std::shared_ptr<Material> &mat, const aiMaterial *aiMat)
    {
        assert(sizeof(ai_real) == sizeof(float));//Ensure assimp is running in single precision mode

		//Get name
        aiString name;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, name)) {
            name = "";
        }
        mat->setName(name.data);

        //Get materials as properties
		aiColor4D colorFactor;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, colorFactor)) {
            colorFactor = aiColor4D(1);
        }
        mat->setColor(*reinterpret_cast<glm::vec4*>(&colorFactor));

        float roughnessFactor;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, colorFactor)) {
            roughnessFactor = 0.0f;
        }
        mat->setRoughness(roughnessFactor);

        float metallicFactor;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor)) {
            metallicFactor = 0.0f;
        }
        mat->setMetallic(metallicFactor);

        aiColor3D emissiveFactor;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, colorFactor)) {
            emissiveFactor = aiColor3D(0);
        }
        mat->setEmissive(*reinterpret_cast<glm::vec3*>(&emissiveFactor));
        
        aiString alphaMode;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode)) {
            fprintf(stderr, "Warning, Material contains unhandled 'AlphaMode' value '%s'\n", alphaMode.C_Str());
        }
        //mat->setAlphaMode();

        float alphaCutOff;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutOff)) {
            alphaCutOff = 0.0f;
        }
        mat->setAlphaCutOff(alphaCutOff);

        bool twoSided;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TWOSIDED, twoSided)) {
            twoSided = false;
        }
        mat->setTwoSided(twoSided);

        //Extension: KHR_materials_pbrSpecularGlossiness
        bool hasSpecGlossy = false;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS, hasSpecGlossy))
        {
            if (hasSpecGlossy)
            {
                aiColor4D diffuseFactor;
                if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseFactor)) {
                    diffuseFactor = aiColor4D(0);
                }
                mat->setDiffuse(*reinterpret_cast<glm::vec4*>(&diffuseFactor));


                aiColor3D specularFactor;
                if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularFactor)) {
                    specularFactor = aiColor3D(0);
                }
                mat->setSpecular(*reinterpret_cast<glm::vec3*>(&specularFactor));

                float glossinessFactor;
                if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS_GLOSSINESS_FACTOR, glossinessFactor)) {
                    if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS, glossinessFactor)) {
                        glossinessFactor = 0.0f;
                    }
                    else
                        glossinessFactor = 1.0f - (glossinessFactor / 1000.0f);//Convert shiny to glossy
                }
                mat->setGlossiness(glossinessFactor);
            }
        }
	}
  //  inline void getMaterialProps(std::shared_ptr<Material> &mat, const aiMaterial *aiMat)
  //  {
		//assert(sizeof(ai_real) == sizeof(float));//Ensure assimp is running in single precision mode

		////Get name
		//aiString name;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, name)) {
		//	name = "";
		//}
		//mat->setName(name.data);

		////Get diffuse colour
		//aiColor3D diffuseColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
		//	diffuseColor = aiColor3D(0,0,0);
		//}
		//mat->setDiffuse(*reinterpret_cast<glm::vec3*>(&diffuseColor));

		////Get specular colour
		//aiColor3D specularColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
		//	specularColor = aiColor3D(0, 0, 0);
		//}
		//mat->setDiffuse(*reinterpret_cast<glm::vec3*>(&specularColor));

		////Get ambient colour
		//aiColor3D ambientColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor)) {
		//	ambientColor = aiColor3D(0, 0, 0);
		//}
		//mat->setAmbient(*reinterpret_cast<glm::vec3*>(&ambientColor));

		////Get transparent colour
		//aiColor3D transparentColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor)) {
		//	transparentColor = aiColor3D(0, 0, 0);
		//}
		//else
		//	fprintf(stderr, "Warning, material '%s' contains unhandled transparent colour!\n", mat->getName().c_str());
		//mat->setTransparent(*reinterpret_cast<glm::vec3*>(&transparentColor));

		////Get emissive colour
		//aiColor3D emissiveColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor)) {
		//	emissiveColor = aiColor3D(0, 0, 0);
		//}
		//else
		//	fprintf(stderr, "Warning, material '%s' contains unhandled emissive colour!\n", mat->getName().c_str());
		////mat->setEmissive(*reinterpret_cast<glm::vec3*>(&emissiveColor));

		////Get reflection colour
		//aiColor3D reflectiveColor;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColor)) {
		//	reflectiveColor = aiColor3D(0, 0, 0);
		//}
		//else
		//	fprintf(stderr, "Warning, material '%s' contains unhandled reflective colour!\n", mat->getName().c_str());

		////Get wireframe property
		//int isWireframe;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_ENABLE_WIREFRAME, isWireframe)) {
		//	isWireframe = 0;
		//}
		//mat->setWireframe(isWireframe != 0);

		////Get isTwosided property
		//int isTwosided;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TWOSIDED, isTwosided)) {
		//	isTwosided = 0;
		//}
		//mat->setTwoSided(isTwosided != 0);

		////Get shadingModel property
		//aiShadingMode shadingModel;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHADING_MODEL, shadingModel)) {
		//	shadingModel = aiShadingMode_Gouraud;
		//}
		//mat->setShadingMode(toShadingMode(shadingModel));
		////Get blendFunc property
		//aiBlendMode blendFunc;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_BLEND_FUNC, blendFunc)) {
		//	blendFunc = aiBlendMode_Default;
		//}
		//if (blendFunc == aiBlendMode_Default)
		//	mat->setAlphaBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//else if (blendFunc == aiBlendMode_Additive)
		//	mat->setAlphaBlendMode(GL_ONE, GL_ONE);
		//else
		//	throw std::runtime_error("Unexpected aiBlendMode value when parsing aiMaterial.");
		//
		////Get opacity property
		//float opacity;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_OPACITY, opacity)) {
		//	opacity = 1.0f;
		//}
		//mat->setOpacity(opacity);

		////Get shininess property
		//float shininess;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS, shininess)) {
		//	shininess = 0.0f;
		//}
		//mat->setShininess(shininess);
		////Get shininess strength property
		//float shininessStrength;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength)) {
		//	shininessStrength = 1.0f;
		//}
		//mat->setShininessStrength(shininessStrength);
		////Get refraction index property
		//float refraction;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_REFRACTI, refraction)) {
		//	refraction = 1.0f;
		//}
		//mat->setRefractionIndex(refraction);
		////Get reflectivity property
		//float reflectivity;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_REFLECTIVITY, reflectivity)) {
		//	reflectivity = 0.0f;
		//}
		//mat->setReflectivity(reflectivity);
  //  }

	inline Material::TextureFrame getTextureProps(const aiTextureType &texType, const aiMaterial *aiMat, unsigned int index = 0, const char *modelFolder = nullptr)
	{
		Material::TextureFrame texFrame;
		//float blend;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXBLEND(texType, index), blend)) {
		//	blend = 1.0f;
		//}
		//texFrame.weight = blend;

		//aiTextureOp op;
		//texFrame.operation = Material::TextureFrame::Null;
		//if (AI_SUCCESS == aiMat->Get(AI_MATKEY_TEXOP(texType, index), op)) {
		//	texFrame.operation = toOp(op);
		//}

		//int mapping;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_MAPPING(texType, index), mapping)) {
		//	mapping = 0;//Something to do with projective coordinates
		//}

		//int UVWSRC;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_UVWSRC(texType, index), UVWSRC)) {
		//	UVWSRC = 0;
		//}
		//texFrame.uvIndex = UVWSRC;

		aiTextureMapMode mappingMode_U;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_MAPPINGMODE_U(texType, index), mappingMode_U)) {
			mappingMode_U = aiTextureMapMode_Wrap;
		}
		texFrame.mappingModeU = toMapMode(mappingMode_U);

		aiTextureMapMode mappingMode_V;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_MAPPINGMODE_V(texType, index), mappingMode_V)) {
			mappingMode_V = aiTextureMapMode_Wrap;
		}
        texFrame.mappingModeV = toMapMode(mappingMode_V);

        SamplerMagFilter magFilter;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_MAPPINGFILTER_MAG(texType, index), magFilter)) {
            magFilter = SamplerMagFilter::UNSET;
        }

        SamplerMinFilter minFilter;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_GLTF_MAPPINGFILTER_MIN(texType, index), minFilter)) {
            minFilter = SamplerMinFilter::UNSET;
        }

		//texFrame.isDecal = ((mappingMode_U&aiTextureFlags_UseAlpha) == aiTextureMapMode_Decal) && ((mappingMode_V&aiTextureFlags_UseAlpha) == aiTextureMapMode_Decal);

		//aiVector3D diffuseTexMapAxis;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXMAP_AXIS(texType, index), diffuseTexMapAxis)) {
		//	//Something to do with sphere tex mapping
		//}
		//texFrame

		//int texFlags;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXFLAGS(texType, index), texFlags)) {
		//	texFlags = 0;
		//}
		//texFrame.invertColor = (texFlags&aiTextureFlags_Invert) == aiTextureFlags_Invert;

		//texFrame.useAlpha = ((texFlags&aiTextureFlags_UseAlpha) == aiTextureFlags_UseAlpha) || !((texFlags&aiTextureFlags_IgnoreAlpha) == aiTextureFlags_IgnoreAlpha);
		aiString path;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXTURE(texType, index), path)) {
			path = "";
		}

        texFrame.texture = Texture2D::load(path.data, modelFolder, toMinFilter(minFilter) | toMagFilter(magFilter) | toMapMode_internal(mappingMode_U, mappingMode_V));
		return texFrame;
	}

};

#endif //__Model_assimpUtils_h__