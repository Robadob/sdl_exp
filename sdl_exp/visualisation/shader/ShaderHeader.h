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
//#pragma push_macro("vec3")
#define vec3 glm::vec3
#endif //#ifdef __cplusplus
    
    struct MaterialProperties
    {
#ifdef __cplusplus
        MaterialProperties()
            : diffuse(0)
            , opacity(1.0f)
            , specular(0)
            , shininess(0)
            , ambient(0)
            , shininessStrength(1.0f)
            , emissive(0)
            , refractionIndex(1.0f)
            , transparent(0)
			, bitmask(0)
        { }
#endif //#ifdef __cplusplus
		vec3 ambient;           //Ambient color
		float opacity;
		vec3 diffuse;           //Diffuse color
		float shininess;
		vec3 specular;          //Specular color
        float shininessStrength;
		vec3 emissive;          //Emissive color (light emitted)
        float refractionIndex;
		vec3 transparent;       //Transparent color, multiplied with translucent light to construct final color
        unsigned int bitmask;   //bitmask to calculate which textures are available
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
			, spotCutoff(180.0f)
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
		float spotCutoff;          // Crli                              
		// (range: [0.0,90.0], 180.0)   
		vec3 specular;             // Scli   
		float spotCosCutoff;       // Derived: cos(Crli)                 
		// (range: [1.0,0.0],-1.0)   
		vec3 position;             // Ppli   
		float constantAttenuation; // K0   
		vec3 halfVector;           // Derived: Hi  (This is calculated as the vector half way between vector-light and vector-viewer) 
		float linearAttenuation;   // K1   
		vec3 spotDirection;        // Sdli   
		float quadraticAttenuation;// K2  
	};
	
#ifdef __cplusplus
#undef vec3
//#pragma pop_macro("vec3")
#endif //#ifdef __cplusplus

const unsigned int MAX_MATERIALS = 50;//Abitrary limit, haven't done maths to solve max in min uniform buffer requirement
const unsigned int MAX_LIGHTS = 50;//Abitrary limit, haven't done maths to solve max in min uniform buffer requirement
#ifndef __cplusplus
//Include in shader
	uniform unsigned int _materialID;
	uniform _materials
	{
		MaterialProperties material[MAX_MATERIALS];
	};	

	uniform _lights
	{
		unsigned int lightsCount;
		//<12 bytes of padding>
		LightProperties light[MAX_LIGHTS];
	};
#endif //#ifndef __cplusplus


#endif //#ifndef __ShaderHeader_h__