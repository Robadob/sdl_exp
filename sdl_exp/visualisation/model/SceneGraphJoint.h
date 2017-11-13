#ifndef __SceneGraphJoint_h__
#define __SceneGraphJoint_h__
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

#include "SceneGraphItem.h"

class SceneGraphJoint;
typedef SceneGraphJoint SGJoint;
/**
 * This class represents an adjustable rotational joint for attaching items to scene graph
 * Add one of these as a child between two renderables
 * You can also do scales and translation, however that's not recommended unless you want scene nodes to be connected by an invisible link
 */
class SceneGraphJoint : public SceneGraphItem
{
public:
	static std::shared_ptr<SceneGraphJoint> make(const glm::mat4 &attachmentTransform = glm::mat4(1));
protected:
	SceneGraphJoint(const glm::mat4 &attachmentTransform = glm::mat4(1));
	/**
	* TODO
	* Can't implement until SceneGraphItem copy constructor is ready
	*/
	SceneGraphJoint(const SceneGraphJoint& b) = delete;
	/**
	* Default behaviour, this object is a simple interface over SceneGraphItem and has no member vars
	*/
	SceneGraphJoint(SceneGraphJoint&& b);
	/**
	 * TODO
	 * Can't implement until SceneGraphItem copy assignment operator is ready
	 */
	SceneGraphJoint& operator= (const SceneGraphJoint& b) = delete;
	/**
	 * Default behaviour, this object is a simple interface over SceneGraphItem and has no member vars
	 */
	SceneGraphJoint& operator= (SceneGraphJoint&& b);
	/**
	* TODO
	* Can't implement until SceneGraphItem copy constructor is ready
	*/
	//std::shared_ptr<SceneGraphJoint> SceneGraphJoint::clone();
	/**
	 * Do nothing
	 * This type is not renderable
	 */
	void render(const glm::mat4 &transform) override { }
public:
	/////////////////////////////////////////////////////////////////////////
	//                 ~Scene Graph Attachment Management~                 //
	// Joints have no attachment points, so we treat it slightly different //
	/////////////////////////////////////////////////////////////////////////
	bool attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, unsigned int childAttachOffsetIndex = 0);
	bool attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, glm::vec3 childAttachOffset);

	//std::shared_ptr<SceneGraphJoint> clone();
    /////////////////////////
    // Matrix Manipulation //
    /////////////////////////
    /**
     * Resets the transformation matrix to the identity matrix
     */
	inline SceneGraphJoint& resetTransform() { setSceneMat(glm::mat4(1)); return *this; }
    /**
     * Replaces the transformation matrix with the provided one
     * @param newTransform The new transform matrix to be used
     */
	inline SceneGraphJoint& setTransform(const glm::mat4 &newTransform) { setSceneMat(newTransform); return *this; }
    /**
     * Scales the existing transformation matrix
     * @param scale The 3-dimensional scale factor to be used
     */
	inline SceneGraphJoint& scale(const glm::vec3 &scale) { setSceneMat(getSceneMatRef() * glm::scale(scale)); return *this; }
    /**
     * Rotates the existing transformation matrix about an axis
     * @param axis The axis to rotate about
     * @param angleDegrees The number of degrees to rotate
     */
	inline SceneGraphJoint& rotate(const float &angleDegrees, const glm::vec3 &axis) { setSceneMat(getSceneMatRef() * glm::rotate(angleDegrees, axis)); return *this; }
    /**
     * Translates the existing transformation matrix
     * @param translation The translation to use
     */
	inline SceneGraphJoint& translate(const glm::vec3 &translation){ setSceneMat(getSceneMatRef() * glm::translate(translation)); return *this; }
    /**
     * Replaces the absolute translation of the transformation matrix
     * @param offset The new absolute translation
     */
	inline SceneGraphJoint& setOffset(const glm::vec3 &offset) {
		glm::mat4 localTransform = getSceneMat();
        localTransform[3][0] = offset.x;
        localTransform[3][1] = offset.y;
        localTransform[3][2] = offset.z;
		setSceneMat(localTransform);
		return *this;
    };
    /////////////////////////////////////
    // Convenience Matrix Manipulation //
    /////////////////////////////////////
    /**
     * Uniformly scales the existing transformation matrix
     * @param scale The scale factor to be used
     */
	inline SceneGraphJoint& scale(const float &scale) { return this->scale(glm::vec3(scale)); }
    /**
     * @see scale(glm::vec3)
     */
	inline SceneGraphJoint& scale(const float &x, const float &y, const float &z) { return this->scale(glm::vec3(x, y, z)); }
    /**
     * @see rotate(glm::vec3, float)
     */
	inline SceneGraphJoint& rotate(const float &angleDegrees, const float &axisX, const float &axisY, const float &axisZ) { return this->rotate(angleDegrees, glm::vec3(axisX, axisY, axisZ)); }
    /**
     * @see translate(glm::vec3)
     */
	inline SceneGraphJoint& translate(const float &x, const float &y, const float &z){ return this->translate(glm::vec3(x, y, z)); }
    /**
     * @see setOffset(glm::vec3)
     */
	inline SceneGraphJoint& setOffset(const float &x, const float &y, const float &z){ return this->setOffset(glm::vec3(x, y, z)); }
};

#endif //__SceneGraphJoint_h__