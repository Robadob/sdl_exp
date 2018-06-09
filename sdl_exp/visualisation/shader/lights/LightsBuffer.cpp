#include "LightsBuffer.h"
#include "PointLight.h"
#include "Spotlight.h"
LightsBuffer::LightsBuffer(const glm::mat4 *viewMatPtr)
	: UniformBuffer(MAX_LIGHTS*sizeof(LightProperties))
	, tProperties()
	, viewMatPtr(viewMatPtr)
{
	uniformBlock.lightsCount = 0;
}
PointLight LightsBuffer::addPointLight()
{
	if (uniformBlock.lightsCount<MAX_LIGHTS)
	{
		unsigned int index = uniformBlock.lightsCount++;
		return PointLight(&uniformBlock.lights[index], &tProperties[index], index);
	}
	throw std::runtime_error("LightsBuffer::addPointLight(): Max lights exceeded in light buffer.\n");
}
SpotLight LightsBuffer::addSpotLight()
{
	if (uniformBlock.lightsCount<MAX_LIGHTS)
	{
		unsigned int index = uniformBlock.lightsCount++;
		return SpotLight(&uniformBlock.lights[index], &tProperties[index], index);
	}
	throw std::runtime_error("LightsBuffer::addSpotLight(): Max lights exceeded in light buffer.\n");
}
PointLight LightsBuffer::getPointLight(unsigned int index)
{
	if (index<uniformBlock.lightsCount)
	{
		return PointLight(&uniformBlock.lights[index], &tProperties[index], index, false);
	}
	throw std::runtime_error("LightsBuffer::getSpotLight(): Light index is invalid.\n");
}
SpotLight LightsBuffer::getSpotLight(unsigned int index)
{
	if (index<uniformBlock.lightsCount)
	{
		return SpotLight(&uniformBlock.lights[index], &tProperties[index], index, false);
	}
	throw std::runtime_error("LightsBuffer::getSpotLight(): Light index is invalid.\n");
}
void LightsBuffer::update()
{
	static bool once = true;
	if (viewMatPtr)
	{
		//Transform light values into eye space
		for (unsigned int i = 0; i < uniformBlock.lightsCount; ++i)
		{
			uniformBlock.lights[i].position = (*viewMatPtr) * tProperties[i].position;
			uniformBlock.lights[i].spotDirection = (*viewMatPtr) * tProperties[i].spotDirection;
		}
	}
	else
	{
		if (once)
		{
			fprintf(stderr, "Warning: viewMatPtr has not been passed to LightsBuffer, lights will not be transformed to eye space.\n");
			once = false;
		}
		for (unsigned int i = 0; i < uniformBlock.lightsCount; ++i)
		{
			uniformBlock.lights[i].position = tProperties[i].position;
			uniformBlock.lights[i].spotDirection = tProperties[i].spotDirection;
		}
	}
	setData(&uniformBlock, sizeof(glm::vec4) + (uniformBlock.lightsCount * sizeof(LightProperties)));//sizeof(LightUniformBlock)
}

//Comment out this include if not making use of Shaders/ShaderCore
#include "../../interface/Renderable.h"
#ifdef __Renderable_h__
void Renderable::setLightsBuffer(std::shared_ptr<LightsBuffer> buffer)
{//Treat it similar to texture binding points
	setLightsBuffer(buffer->getBufferBindPoint());
}
#endif