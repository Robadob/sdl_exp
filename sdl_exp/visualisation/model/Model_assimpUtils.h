#ifndef __Model_assimpUtils_h__
#define __Model_assimpUtils_h__

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../texture/Texture2D.h"

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
		if (t == aiTextureType_DIFFUSE)
			return Material::TextureType::Diffuse;
		if (t == aiTextureType_SPECULAR)
			return Material::TextureType::Specular;
		if (t == aiTextureType_AMBIENT)
			return Material::TextureType::Ambient;
		if (t == aiTextureType_EMISSIVE)
			return Material::TextureType::Emmisive;
		if (t == aiTextureType_HEIGHT)
			return Material::TextureType::Height;
		if (t == aiTextureType_NORMALS)
			return Material::TextureType::Normals;
		if (t == aiTextureType_SHININESS)
			return Material::TextureType::Shininess;
		if (t == aiTextureType_OPACITY)
			return Material::TextureType::Opacity;
		if (t == aiTextureType_DISPLACEMENT)
			return Material::TextureType::Displacement;
		if (t == aiTextureType_LIGHTMAP)
			return Material::TextureType::LightMap;
		if (t == aiTextureType_REFLECTION)
			return Material::TextureType::Reflection;
		if (t == aiTextureType_UNKNOWN)
			return Material::TextureType::Unknown;
		if (t == aiTextureType_NONE)
			return Material::TextureType::None;
		throw std::runtime_error("Unexpected aiTextureType value when parsing aiMaterial.");
	}
	inline Material::TextureFrame::BlendOperation toOp(const aiTextureOp &op)
	{
		if (op == aiTextureOp_Multiply)
			return Material::TextureFrame::Multiply;
		if (op == aiTextureOp_Add)
			return Material::TextureFrame::Add;
		if (op == aiTextureOp_Subtract)
			return Material::TextureFrame::Subtract;
		if (op == aiTextureOp_Divide)
			return Material::TextureFrame::Divide;
		if (op == aiTextureOp_SmoothAdd)
			return Material::TextureFrame::SmoothAdd;
		if (op == aiTextureOp_SignedAdd)
			return Material::TextureFrame::SignedAdd;
		throw std::runtime_error("Unexpected aiTextureOp value when parsing aiMaterial.");
	}
	inline GLuint toMapMode(aiTextureMapMode &mapMode)
	{
		if (mapMode == aiTextureMapMode_Wrap)
			return GL_REPEAT;
		if (mapMode == aiTextureMapMode_Clamp)
			return GL_CLAMP_TO_EDGE;
		if (mapMode == aiTextureMapMode_Decal)
			return GL_CLAMP_TO_EDGE;//Want border to be transparent here!
		if (mapMode == aiTextureMapMode_Mirror)
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
	inline Material::ShadingMode toShadingMode(const aiShadingMode &mode)
	{
		if (mode == aiShadingMode_Flat)
			return Material::ShadingMode::Flat;
		if (mode == aiShadingMode_Gouraud)
			return Material::ShadingMode::Gouraud;
		if (mode == aiShadingMode_Phong)
			return Material::ShadingMode::Phong;
		if (mode == aiShadingMode_Blinn)
			return Material::ShadingMode::BlinnPhong;
		if (mode == aiShadingMode_Toon)
			return Material::ShadingMode::Toon;
		if (mode == aiShadingMode_OrenNayar)
			return Material::ShadingMode::OrenNayar;
		if (mode == aiShadingMode_Minnaert)
			return Material::ShadingMode::Minnaert;
		if (mode == aiShadingMode_CookTorrance)
			return Material::ShadingMode::CookTorrance;
		if (mode == aiShadingMode_NoShading)
			return Material::ShadingMode::Constant;
		if (mode == aiShadingMode_Fresnel)
			return Material::ShadingMode::Fresnel;
		throw std::runtime_error("Unexpected aiShadingMode value when parsing aiMaterial.");
	}
	inline void getMaterialProps(std::shared_ptr<Material> &mat, const aiMaterial *aiMat)
    {
		assert(sizeof(ai_real) == sizeof(float));//Ensure assimp is running in single precision mode

		//Get name
		aiString name;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, name)) {
			name = "";
		}
		mat->setName(name.data);

		//Get diffuse colour
		aiColor3D diffuseColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
			diffuseColor = aiColor3D(0,0,0);
		}
		mat->setDiffuse(*reinterpret_cast<glm::vec3*>(&diffuseColor));

		//Get specular colour
		aiColor3D specularColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
			specularColor = aiColor3D(0, 0, 0);
		}
		mat->setDiffuse(*reinterpret_cast<glm::vec3*>(&specularColor));

		//Get ambient colour
		aiColor3D ambientColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor)) {
			ambientColor = aiColor3D(0, 0, 0);
		}
		mat->setAmbient(*reinterpret_cast<glm::vec3*>(&ambientColor));

		//Get transparent colour
		aiColor3D transparentColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor)) {
			transparentColor = aiColor3D(0, 0, 0);
		}
		else
			fprintf(stderr, "Warning, material '%s' contains unhandled transparent colour!\n", mat->getName().c_str());
		mat->setTransparent(*reinterpret_cast<glm::vec3*>(&transparentColor));

		//Get emissive colour
		aiColor3D emissiveColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor)) {
			emissiveColor = aiColor3D(0, 0, 0);
		}
		else
			fprintf(stderr, "Warning, material '%s' contains unhandled emissive colour!\n", mat->getName().c_str());
		//mat->setEmissive(*reinterpret_cast<glm::vec3*>(&emissiveColor));

		//Get reflection colour
		aiColor3D reflectiveColor;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColor)) {
			reflectiveColor = aiColor3D(0, 0, 0);
		}
		else
			fprintf(stderr, "Warning, material '%s' contains unhandled reflective colour!\n", mat->getName().c_str());

		//Get wireframe property
		int isWireframe;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_ENABLE_WIREFRAME, isWireframe)) {
			isWireframe = 0;
		}
		mat->setWireframe(isWireframe != 0);

		//Get isTwosided property
		int isTwosided;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TWOSIDED, isTwosided)) {
			isTwosided = 0;
		}
		mat->setTwoSided(isTwosided != 0);

		//Get shadingModel property
		aiShadingMode shadingModel;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHADING_MODEL, shadingModel)) {
			shadingModel = aiShadingMode_Gouraud;
		}
		mat->setShadingMode(toShadingMode(shadingModel));
		//Get blendFunc property
		aiBlendMode blendFunc;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_BLEND_FUNC, blendFunc)) {
			blendFunc = aiBlendMode_Default;
		}
		if (blendFunc == aiBlendMode_Default)
			mat->setAlphaBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (blendFunc == aiBlendMode_Additive)
			mat->setAlphaBlendMode(GL_ONE, GL_ONE);
		else
			throw std::runtime_error("Unexpected aiBlendMode value when parsing aiMaterial.");
		
		//Get opacity property
		float opacity;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_OPACITY, opacity)) {
			opacity = 1.0f;
		}
		mat->setOpacity(opacity);

		//Get shininess property
		float shininess;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS, shininess)) {
			shininess = 0.0f;
		}
		mat->setShininess(shininess);
		//Get shininess strength property
		float shininessStrength;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength)) {
			shininessStrength = 1.0f;
		}
		mat->setShininessStrength(shininessStrength);
		//Get refraction index property
		float refraction;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_REFRACTI, refraction)) {
			refraction = 1.0f;
		}
		mat->setRefractionIndex(refraction);
		//Get reflectivity property
		float reflectivity;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_REFLECTIVITY, reflectivity)) {
			reflectivity = 0.0f;
		}
		mat->setReflectivity(reflectivity);
    }

	inline Material::TextureFrame getTextureProps(const aiTextureType &texType, const aiMaterial *aiMat, unsigned int index = 0, const char *modelFolder = nullptr)
	{
		Material::TextureFrame texFrame;
		float blend;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXBLEND(texType, index), blend)) {
			blend = 1.0f;
		}
		texFrame.weight = blend;

		aiTextureOp op;
		texFrame.operation = Material::TextureFrame::Null;
		if (AI_SUCCESS == aiMat->Get(AI_MATKEY_TEXOP(texType, index), op)) {
			texFrame.operation = toOp(op);
		}

		//int mapping;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_MAPPING(texType, index), mapping)) {
		//	mapping = 0;//Something to do with projective coordinates
		//}

		int UVWSRC;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_UVWSRC(texType, index), UVWSRC)) {
			UVWSRC = 0;
		}
		texFrame.uvIndex = UVWSRC;

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

		texFrame.isDecal = ((mappingMode_U&aiTextureFlags_UseAlpha) == aiTextureMapMode_Decal) && ((mappingMode_V&aiTextureFlags_UseAlpha) == aiTextureMapMode_Decal);

		//aiVector3D diffuseTexMapAxis;
		//if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXMAP_AXIS(texType, index), diffuseTexMapAxis)) {
		//	//Something to do with sphere tex mapping
		//}
		//texFrame

		int texFlags;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXFLAGS(texType, index), texFlags)) {
			texFlags = 0;
		}
		texFrame.invertColor = (texFlags&aiTextureFlags_Invert) == aiTextureFlags_Invert;

		texFrame.useAlpha = ((texFlags&aiTextureFlags_UseAlpha) == aiTextureFlags_UseAlpha) || !((texFlags&aiTextureFlags_IgnoreAlpha) == aiTextureFlags_IgnoreAlpha);
		aiString path;
		if (AI_SUCCESS != aiMat->Get(AI_MATKEY_TEXTURE(texType, index), path)) {
			path = "";
		}

		texFrame.texture = Texture2D::load(path.data, modelFolder, Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR | Texture::FILTER_MAG_LINEAR | toMapMode_internal(mappingMode_U, mappingMode_V));
		return texFrame;
	}

};

#endif //__Model_assimpUtils_h__