#ifndef __Axis_h__
#define __Axis_h__
#include "interface/Renderable.h"
#include "Draw.h"

class Axis : public Renderable
{
public:
	/**
	 * Allocates buffer objects for the vertices/colors/face-indices of axis marker of the given length
	 * @param length The length of each axis marker line
	 */
    Axis(float length = 1.0);
	/**
	 * Renders a simple axis marker. Red displays the positive x, Green the positive y and Blue the positive z.
	 */
    void render();
	/**
	 * Reloads the shader
	 */
	void reload() override;
	/**
	 * Provides view matrix to the shader
	 */
	void setViewMatPtr(glm::mat4 const *viewMat) override;
	/**
	* Provides projection matrix to the shader
	*/
	void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
	/**
	* Provides lights buffer to the shader
	* @param bufferBindingPoint Set the buffer binding point to be used for rendering
	*/
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;

    std::shared_ptr<PortableDraw> makePortable() { return pen.makePortable(DRAWING_NAME); }
private:
    Draw pen;
    static const char *DRAWING_NAME;
};

#endif //ifndef __Axis_h__