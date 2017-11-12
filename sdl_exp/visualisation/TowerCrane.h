#ifndef __TowerCrane_h__
#define __TowerCrane_h__
#include "model/SceneGraphItem.h"
#include "Entity.h"

/**
 * This class wraps a scene graph for a basic animated tower crane
 */
class TowerCrane : public SceneGraphItem, public Renderable
{
public:
	static std::shared_ptr<TowerCrane> make(glm::vec3 location = glm::vec3(0), float scale = 1.0f);
	void render(const glm::mat4 &transform) override { }//Do nothing, root is not renderable
private:
	TowerCrane(glm::vec3 location, float scale = 1.0f);
	void setViewMatPtr(glm::mat4 const *viewMat) override;
	void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
private:
	void setupCraneSceneGraph();
	std::shared_ptr<Entity> craneBase, craneTower, craneCounterWeight, craneJib, craneTrolley;
	std::shared_ptr<SceneGraphJoint> spinJoint, trolleySlideJoint;
	const float scale;
	const glm::vec3 location;
	using SceneGraphItem::attach;
};

#endif //__TowerCrane_h__