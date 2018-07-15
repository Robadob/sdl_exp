#ifndef __ShaderHeader_h__
#define __ShaderHeader_h__
/**
 * This header provides structs for use within shaders
 * The __cplusplus ifdefs, exist to allow inclusion in cpp code for binding the shaders
 * Structures include padding and member order to ensure that nothing crosses 4 byte lines
 * @note The returned classes contain raw pointers to the parents UniformBuffer 
 * and therefore should not exist after it has been destroyed
 */
#ifdef __cplusplus
#include <glm/glm.hpp>
#define vec3 glm::vec3
#define vec4 glm::vec4
#define uint glm::uint
#endif //#ifdef __cplusplus

struct MaterialProperties
    {
#ifdef __cplusplus
		MaterialProperties()
          : color(1) //White
          , roughness(0)
          , metallic(0)
          , refractionIndex(1.0f) //Air
          , alphaCutOff(0)
          , emissive(0)
          , bitmask(0)
          , diffuse(0)
          , specular(0)
          , glossiness(0)
    {
    }
#endif //#ifdef __cplusplus
		vec4 color;             //Base color (aka diffuse, albedo)
        float roughness;
        float metallic;
        float refractionIndex;	//Unused (padding)
        float alphaCutOff;
        vec3 emissive;          //Unused, Emissive color (light emitted) (disabled to keep struct size down temporary
        uint bitmask;           //bitmask to calculate which textures are available
        //Extension: KHR_materials_pbrSpecularGlossiness
		vec4 diffuse;           //Diffuse reflection color
		vec3 specular;          //Specular reflection color
        float glossiness;       //Glossiness factor
    };

	struct LightProperties
	{
#ifdef __cplusplus
		/**
		 * Using OpenGL default values
		 * White light (with no ambient component)
		 */
		LightProperties()
			: ambient(0)
			, spotExponent(0)
			, diffuse(1)
			, PADDING1(0)
			, specular(1)
			, spotCosCutoff(cos(180.0f))
			, position(0,0,1)
			, constantAttenuation(1)
			, halfVector(0)
			, linearAttenuation(0)
			, spotDirection(0,0,-1)
			, quadraticAttenuation(0)
		{ }
#endif //#ifdef __cplusplus
		vec3 ambient;              // Aclarri   
		float spotExponent;        // Srli   
		vec3 diffuse;              // Dcli   
		float PADDING1;            // Crli   (ex spot cutoff, this value is nolonger set internally)
		vec3 specular;             // Scli   
		float spotCosCutoff;       // Derived: cos(Crli) // (Valid spotlight range: [1.0,0.0]), negative == pointlight, greater than 1.0 == directional light
		vec3 position;             // Ppli   
		float constantAttenuation; // K0   
		vec3 halfVector;           // Derived: Hi  (This is calculated as the vector half way between vector-light and vector-viewer) 
		float linearAttenuation;   // K1   
		vec3 spotDirection;        // Sdli   
		float quadraticAttenuation;// K2  
	};
	

	const uint MAX_MATERIALS = 50;//Abitrary limit, haven't done maths to solve max in min uniform buffer requirement
	const uint MAX_LIGHTS = 50;//Abitrary limit, haven't done maths to solve max in min uniform buffer requirement

#ifndef __cplusplus
//Include in shader
	uniform uint _materialID;
	uniform _materials
	{
		MaterialProperties material[MAX_MATERIALS];
	};	

	uniform _lights
	{
		uint lightsCount;
		//<12 bytes of padding>
		LightProperties light[MAX_LIGHTS];
	};

	uniform sampler2D t_ambient;
	uniform sampler2D t_diffuse;
	uniform sampler2D t_specular;
#else
	struct LightUniformBlock
	{
		unsigned int lightsCount;
		unsigned int padding1;
		unsigned int padding2;
		unsigned int padding3;
		LightProperties lights[MAX_LIGHTS];
	};
#endif //#ifndef __cplusplus
	
#ifdef __cplusplus
#undef uint
#undef vec3
#undef vec4
#endif //#ifdef __cplusplus

#endif //#ifndef __ShaderHeader_h__