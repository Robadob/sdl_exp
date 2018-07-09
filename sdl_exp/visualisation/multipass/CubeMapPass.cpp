#include "CubeMapPass.h"
#include "../shader/lights/LightsBuffer.h"

CubeMapPass::CubeMapPass(
	std::shared_ptr<CubeMapFrameBuffer> fb,
	std::shared_ptr<RenderPass> childPass,
	std::shared_ptr<RenderableAdv> ent,
	Visualisation &visualisation,
	std::shared_ptr<LightsBuffer> lighting
	)
	: RenderPass(fb)
	, cubeMap(fb)
	, childPass(childPass)
	, ent(ent)
	, visualisation(visualisation)
	, lighting(lighting)
{
	
}

void CubeMapPass::render()
{
	if (ent->visible())
	{
		//Set source model to not visible
		ent->visible(false);
		//Override projection matrix
		visualisation.setProjectionMat(CubeMapFrameBuffer::getProjecitonMat());
		//For each face of cube map framebuffer
		for (unsigned int i = 0; i < 6; ++i)
		{
			CubeMapFrameBuffer::Face f = CubeMapFrameBuffer::Face(i);
			//Use cube map face
			cubeMap->use(f);
			//Overrides view matrix
			this->visualisation.getCamera()->setViewMat(CubeMapFrameBuffer::getViewMat(f, ent->getLocation()));
			this->visualisation.getCamera()->setSkyBoxViewMat(CubeMapFrameBuffer::getSkyBoxViewMat(f));
			//Update lights
			lighting->update();
			//Render scene
			childPass->render();
		}
		//Update mipmap (I hate this step needing to be manually called)
		cubeMap->updateMipMap();
		//Reset framebuffer (implicitly handled at next render pass)
		//Reset matricies
		visualisation.resetProjectionMat();
		visualisation.getCamera()->resetViewMats();
		//Update lights
		lighting->update();
		//Reset source model to visible
		ent->visible(true);
	}
}
