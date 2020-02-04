#ifndef __TextBillboard_h__
#define __TextBillboard_h__

#include "../visualisation/interface/Renderable.h"
#include "../visualisation/shader/Shaders.h"
#include "../visualisation/camera/NoClipCamera.h"
#include "../visualisation/Text.h"

class TextBillboard : public Renderable
{
public:
    TextBillboard(std::shared_ptr<const NoClipCamera> camera, const std::string &string);
    ~TextBillboard();
    void reload() override;
    void render(const glm::vec3 &translation);
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
    glm::vec3 translation;
    std::shared_ptr<Shaders> billboardShaders;
    void *billboardData;
    GLuint billboardVBO, billboardFVBO;
    std::shared_ptr<Text> text;
    glm::mat4 modelMat;
};

#endif  // __TextBillboard_h__