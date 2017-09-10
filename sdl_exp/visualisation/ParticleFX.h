#ifndef __ParticleFX_h__
#define __ParticleFX_h__
#include "shader/Sort.h"
#include <memory>
#include "interface/Renderable.h"
#include "shader/Shaders.h"

/**
* Provides convenient compute shader Sort functionality
* Currently specialised to sorting particles back to front
* @todo In future hope to generalise this to 1/2/3 dimensional items and custom conditions via templated shaders or similar
*/
class ParticleFX : public Renderable
{
public:
	/**
	 * 
	 */
	ParticleFX(const Camera *camera);
	~ParticleFX();
	void render() const;
	void reload() override;
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
private:
	std::unique_ptr<Sort> particleSort;
	std::unique_ptr<Shaders> billboardShaders;
	GLuint particleLocationBuf;
	const unsigned int PARTICLE_COUNT;
	void *billboardData;
	GLuint billboardVBO, billboardFVBO;
};

#endif //__ParticleFX_h__