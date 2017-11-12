#ifndef __Axis_h__
#define __Axis_h__
#include "interface/Renderable.h"
#include "shader/Shaders.h"
#include <memory>

class Axis : public Renderable, public Reloadable
{
public:
	/**
	 * Allocates buffer objects for the vertices/colors/face-indices of axis marker of the given length
	 * @param length The length of each axis marker line
	 */
    Axis(float length = 1.0);
	/**
	 * Frees buffer objects allocated by constructor
	 */
    ~Axis();
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
private:
	GLuint vbo, fvbo;
	Shaders::VertexAttributeDetail vertices, colors, faces;
	std::shared_ptr<Shaders> shaders;
};

#endif //ifndef __Axis_h__