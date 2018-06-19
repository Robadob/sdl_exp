#ifndef __DirectionalLight_h__
#define __DirectionalLight_h__
#include "PointLight.h"

/**
 * Denotes a directional light by setting spotCosCutoff to invalid range spotCosCutoff>1
 * Stores direction in spotDirection, ignores position
 */
class DirectionalLight : public PointLight
{
protected:
	friend class LightsBuffer;
	inline DirectionalLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init = true);
public:
	inline DirectionalLight(const PointLight &old);
	/**
	 * The direction that the light faces
	 * This automatically normalises the value
	 */
	inline void Direction(const glm::vec3 &dir);
	glm::vec3 Direction() const { return tProperties->spotDirection; }
private:
	//Directional light needs no position, or fancy attenuation (as it has no position)
	using PointLight::Position;
	using PointLight::LinearAttenuation;
	using PointLight::QuadraticAttenuation;
};
#include "DirectionalLight.imp"
#endif //__DirectionalLight_h__