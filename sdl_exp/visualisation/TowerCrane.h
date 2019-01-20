#ifndef __TowerCrane_h__
#define __TowerCrane_h__
#include "model/SceneGraphItem.h"
#include "Entity.h"
#include "model/SlideJoint.h"

/**
 * This class wraps a scene graph for a basic animated tower crane
 */
class TowerCrane : public SceneGraphItem, public Renderable
{
public:
    static std::shared_ptr<TowerCrane> make(glm::vec3 location = glm::vec3(0), float scale = 1.0f);
    void render(const unsigned int &shaderIndex, const glm::mat4 &transform) override { }//Do nothing, root is not renderable
    void setViewMatPtr(glm::mat4 const *viewMat) override;
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
    void update(unsigned int frameTime);
    void reload() override; 
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;

    void setJibRotation(const float &rads);
    void rotateJib(const float &rads, const bool &limitSpeed = true);
    void setTrolleyPosition(const float &pos);
    void slideTrolley(const float &offset, const bool &limitSpeed = true);
private:
    TowerCrane(glm::vec3 location, float scale = 1.0f);
private:
    void setupCraneSceneGraph();
    std::shared_ptr<Entity> craneBase, craneTower, craneCounterWeight, craneJib, craneTrolley;
    std::shared_ptr<SceneGraphJoint> spinJoint;
    std::shared_ptr<SlideJoint> trolleySlideJoint;
    const float scale;
    const glm::vec3 location;
    using SceneGraphItem::attach;
    float jibRot, trolleyOffset;
};

#endif //__TowerCrane_h__