#ifndef __CubeMapPass_h__
#define __CubeMapPass_h__
#include "RenderPass.h"
#include "CubeMapFrameBuffer.h"
#include "../interface/Renderable.h"
#include "../Visualisation.h"

class CubeMapPass : public RenderPass
{
public:
	CubeMapPass(
		std::shared_ptr<CubeMapFrameBuffer> fb,
		std::shared_ptr<RenderPass> childPass,
		std::shared_ptr<RenderableAdv> ent,
		Visualisation &visualisation,
		std::shared_ptr<LightsBuffer> lighting,
		const glm::vec3 &originOffset = glm::vec3(0)
		);
protected:
	void render() override;
private:
	std::shared_ptr<CubeMapFrameBuffer> cubeMap;
	std::shared_ptr<RenderPass> childPass;
	std::shared_ptr<RenderableAdv> ent;
	Visualisation &visualisation;
	std::shared_ptr<LightsBuffer> lighting;
	glm::vec3 originOffset;
};

#endif //__CubeMapPass_h__