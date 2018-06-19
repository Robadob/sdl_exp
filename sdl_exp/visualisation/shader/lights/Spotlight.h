#ifndef __SpotLight_h__
#define __SpotLight_h__
#include "PointLight.h"

/**
 * Denotes spotlight by setting spotCosCutoff to valid range 0<=spotCosCutoff<=1
 */
class SpotLight : public PointLight
{
protected:
	friend class LightsBuffer;
	inline SpotLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init = true);
public:
	inline SpotLight(const PointLight &old);
	/**
	 * The direction that the spotlight faces
	 * This automatically normalises the value
	 */
	inline void Direction(const glm::vec3 &dir);
	glm::vec3 Direction() const { return tProperties->spotDirection; }
	/**
	 * The angle of the spotlight's beam in degrees
	 * This value is valid from 0-90 inclusive
	 */
	inline void CutOff(const float &degrees);
	float CutOff() const { return tProperties->spotCutoff; }
	float CosCutOff() const { return properties->spotCosCutoff; }
	/**
	 * The light's intensity is highest in the center of the cone.
	 * It's attenuated toward the edges of the cone by the cosine of the angle between
	 * the direction of the light and the direction from the light to the vertex being lit,
	 * raised to the power of the spot exponent.
	 * Thus, higher spot exponents result in a more focused light source.
	 */
	inline void Exponent(const float &exponent);
	float Exponent() const { return properties->spotExponent; }
};
#include "SpotLight.imp"
#endif //__SpotLight_h__