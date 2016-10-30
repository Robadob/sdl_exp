#include "RenderPass.h"

RenderPass::RenderPass(glm::vec3 clearColor, bool doClear)
	: fb(nullptr)
{ }

RenderPass::RenderPass(std::shared_ptr<FBuffer> fb)
	: fb(fb)
{ }
void RenderPass::executeRender()
{
	//Bind back buffer if shared ptr is null
	if (fb)
	{
		//Skip render if Framebuffer is incomplete
		if (!fb->use())
			return;
	}
	else
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	render();
}
void RenderPass::resize(int width, int height)
{
	if (fb)
		fb->resize(width, height);
}