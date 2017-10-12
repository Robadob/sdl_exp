#include "RenderPass.h"
#include "BackBuffer.h"

RenderPass::RenderPass(const bool &doClear)
    : fb(std::make_shared<BackBuffer>(doClear, glm::vec3(-1)))
{
    
}
RenderPass::RenderPass(const glm::vec3 &clearColor)
    : fb(std::make_shared<BackBuffer>(true, clearColor))
{
}

RenderPass::RenderPass(std::shared_ptr<FBuffer> fb)
	: fb(fb)
{ }
void RenderPass::executeRender()
{
	//Skip render if Framebuffer is incomplete
	if (!fb->use())
		return;
	render();
}
void RenderPass::resize(const glm::uvec2 &dims)
{
	if (!std::dynamic_pointer_cast<BackBuffer>(fb))
		fb->resize(dims);
}