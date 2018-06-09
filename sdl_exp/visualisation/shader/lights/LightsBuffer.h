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
	/**
	 * Struct to hold temp properties
	 */
	struct TLightProperties
	{
		TLightProperties()
			: spotCutoff(180.0f)
			, position(0, 0, 1, 1)
			, spotDirection(0, 0, -1, 0)
		{
			
		}
		float spotCutoff;
		glm::vec4 position;
		glm::vec4 spotDirection;
	};
	LightsBuffer(const glm::mat4 *viewMatPtr = nullptr);
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
	 * Asks the buffer to update the GPU copy if necessary
	 */
	void update();
	/**
	 * Returns the number of allocated lights
	 */
	unsigned int getCount() const { return uniformBlock.lightsCount; }
	/**
	 * Returns the maximum number of lights that can be allocated
	 */
	static unsigned int getMaxCount() { return MAX_LIGHTS; }
	using BufferCore::getName;
	using BufferCore::getType;
	using BufferCore::getBufferBindPoint;
	/**
	* Sets the pointer from which the View matrix should be loaded from
	* @param viewMat A pointer to the viewMatrix to be tracked
	* @note This pointer is likely provided by a Camera subclass
	*/
	void setViewMatPtr(const glm::mat4  *viewMat) { viewMatPtr = viewMat; };
private:
	LightUniformBlock uniformBlock;
	TLightProperties tProperties[MAX_LIGHTS];
	const glm::mat4 *viewMatPtr;
};

#include "PointLight.h"
#include "Spotlight.h"

#endif //__LightsBuffer_h__