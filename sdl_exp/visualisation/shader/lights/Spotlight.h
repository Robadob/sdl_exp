#ifndef __SpotLight_h__
#define __SpotLight_h__
#include "PointLight.h"

class SpotLight : public PointLight
{
protected:
	friend class LightsBuffer;
	inline SpotLight(LightsBuffer *buffer, LightProperties *props, unsigned int index);
public:
	inline SpotLight(const PointLight &old);
	/**
	 * The direction that the spotlight faces
	 */
	inline void Direction(const glm::vec3 &dir);
	glm::vec3 Direction() const { return properties->spotDirection; }
	/**
	 * The angle of the spotlight's beam in degrees
	 * This value is valid from 0-90 inclusive
	 */
	inline void CutOff(const float &degrees);
	float CutOff() const { return properties->spotCutoff; }
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
#include "Spotlight.imp"
#endif //__SpotLight_h__