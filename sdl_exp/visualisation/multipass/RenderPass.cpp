#include "RenderPass.h"

RenderPass::RenderPass(glm::vec3 clearColor, bool doClear)
	: fb()
	//, clearColor(clearColor)
	//, doClear(doClear)
	, scale(0)
	, dimensions(dimensions)
{ }

RenderPass::RenderPass(std::shared_ptr<FrameBuffer> fb, float scale, glm::vec3 clearColor, bool doClear)
	: fb(fb)
	//, clearColor(clearColor)
	//, doClear(doClear)
	, scale(scale)
{ }