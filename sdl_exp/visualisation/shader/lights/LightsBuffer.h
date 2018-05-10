#ifndef __LightsBuffer_h__
#define __LightsBuffer_h__
#include "../buffer/UniformBuffer.h"
#include "../ShaderHeader.h"

class PointLight;
class SpotLight;

/**
 * This class acts as a wrapper over Uniform Buffer which stores lights
 * You can add and get existing lights via the contained methods
 * It is recommended that you store the returned lights in your own data structure
 * They can be created and cast via the copy constructor, but will always refer to their index in the parent LightsBuffer
 */
class LightsBuffer : private UniformBuffer
{
public:
	LightsBuffer();
	~LightsBuffer();
	/**
	 * Increments the number of lights and returns the newest light as a point light
	 * @throws runtime_error If the number of lights would exceed MAX_LIGHTS
	 */
	PointLight addPointLight();
	/**
	 * Increments the number of lights and returns the newest light as a spot light
	 * @throws runtime_error If the number of lights would exceed MAX_LIGHTS
	 */
	SpotLight addSpotLight();
	/**
	 * Returns the light at the given index in the buffer as a point light
	 * @throws runtime_error if index does not point to a current light
	 * @note If you request a light which has already been created as a spotlight
	 * you will simply lose access to spotlight specific variables via this instance
	 */
	PointLight getPointLight(unsigned int index);
	/**
	 * Returns the light at the given index in the buffer as a spot light
	 * @throws runtime_error if index does not point to a current light
	 */
	SpotLight getSpotLight(unsigned int index);
	/**
	 * Tells the buffer that internal data has been changed, so that it will update the GPU copy of the buffer
	 */
	void HasChanged() { hasChanged = true; }
	/**
	 * Asks the buffer to update the GPU copy if necessary
	 */
	void update();
	/**
	 * Returns the number of allocated lights
	 */
	unsigned int getCount() const { return lightsCount; }
	/**
	 * Returns the maximum number of lights that can be allocated
	 */
	static unsigned int getMaxCount() { return MAX_LIGHTS; }
	using BufferCore::getName;
	using BufferCore::getType;
	using BufferCore::getBufferBindPoint;
private:
	void *data;
	unsigned int &lightsCount;
	LightProperties* const lights;
	bool hasChanged;
};

#include "PointLight.h"
#include "Spotlight.h"

#endif //__LightsBuffer_h__