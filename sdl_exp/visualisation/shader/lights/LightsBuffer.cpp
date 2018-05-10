#include "LightsBuffer.h"
#include "PointLight.h"
#include "Spotlight.h"
LightsBuffer::LightsBuffer()
	: UniformBuffer(MAX_LIGHTS*sizeof(LightProperties))
	, data(malloc((4*sizeof(unsigned int))+MAX_LIGHTS*sizeof(LightProperties)))
	, lightsCount(reinterpret_cast<unsigned int *>(data)[0])
	, lights(reinterpret_cast<LightProperties *>(&reinterpret_cast<unsigned int *>(data)[4]))
	, hasChanged(false)
{
	lightsCount = 0;
}
LightsBuffer::~LightsBuffer()
{
	free(data);
}
PointLight LightsBuffer::addPointLight()
{
	if (lightsCount<MAX_LIGHTS)
	{
		lightsCount++;
		return PointLight(this, &lights[lightsCount - 1], lightsCount - 1);
	}
	throw std::runtime_error("LightsBuffer::addPointLight(): Max lights exceeded in light buffer.\n");
}
SpotLight LightsBuffer::addSpotLight()
{
	if (lightsCount<MAX_LIGHTS)
	{
		lightsCount++;
		return SpotLight(this, &lights[lightsCount - 1], lightsCount - 1);
	}
	throw std::runtime_error("LightsBuffer::addSpotLight(): Max lights exceeded in light buffer.\n");
}
PointLight LightsBuffer::getPointLight(unsigned int index)
{
	if (index<lightsCount)
	{
		return PointLight(this, &lights[index], index);
	}
	throw std::runtime_error("LightsBuffer::getSpotLight(): Light index is invalid.\n");
}
SpotLight LightsBuffer::getSpotLight(unsigned int index)
{
	if (index<lightsCount)
	{
		return SpotLight(this, &lights[index], index);
	}
	throw std::runtime_error("LightsBuffer::getSpotLight(): Light index is invalid.\n");
}
void LightsBuffer::update()
{
	if (hasChanged)
	{
		setData(data, sizeof(unsigned int) + MAX_LIGHTS*sizeof(LightProperties));
	}
}

//Comment out this include if not making use of Shaders/ShaderCore
#include "../../interface/Renderable.h"
#ifdef __Renderable_h__
void Renderable::setLightsBuffer(std::shared_ptr<LightsBuffer> buffer)
{//Treat it similar to texture binding points
	setLightsBuffer(buffer->getBufferBindPoint());
}
#endif