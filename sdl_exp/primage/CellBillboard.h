#ifndef __CellBillboard_h__
#define __CellBillboard_h__

#include <memory>
#include "../visualisation/interface/Renderable.h"
#include "../visualisation/shader/Shaders.h"
#include "../visualisation/camera/NoClipCamera.h"

/**
* Provides convenient compute shader Sort functionality
* Currently specialised to sorting particles back to front
* @todo In future hope to generalise this to 1/2/3 dimensional items and custom conditions via templated shaders or similar
*/
class CellBillboard : public Renderable
{
public:
	/**
	*
	*/
	CellBillboard(std::shared_ptr<const NoClipCamera> camera);
	~CellBillboard();
	void reload() override;
	void renderInstances(int count);
	/**
	* Binds the provided modelview matrix to the internal shader
	* @param viewMat Ptr to modelview matrix
	* @note This is normally found within the Camera object
	*/
	void setViewMatPtr(glm::mat4 const *viewMat) override;
	/**
	* Binds the provided projection matrix to the internal shader
	* @param projectionMat Ptr to model view matrix
	* @note This is normally found within the Viewport object
	*/
	void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
	std::shared_ptr<Shaders> getShaders() { return billboardShaders; }

	void setLightsBuffer(const GLuint &bufferBindingPoint) override;

private:
	std::shared_ptr<Shaders> billboardShaders;
	void *billboardData;
	GLuint billboardVBO, billboardFVBO;
	glm::mat4 modelMat;
};

#endif //__CellBillboard_h__