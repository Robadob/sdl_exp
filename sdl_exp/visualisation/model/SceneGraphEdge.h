#ifndef __SceneGraphEdge_h__
#define __SceneGraphEdge_h__
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <list>

#include "SceneGraphVertex.h"

/**
 * This class represents an edge in a scene graph
 * These recursively combine to produce a hierarchy
 * The edge holds a sceneGraph transformation and an associated SceneGraphVertex
 */
class SceneGraphEdge
{
	friend class SceneGraphVertex; //Accesses private constructor
    typedef SceneGraphVertex SGEdge;
private:
    /**
     * Constructor, only to be called by SceneGraphVertex
     */
	SceneGraphEdge(const std::shared_ptr<SceneGraphVertex> &vertex, const glm::mat4 &attachmentTransform);
	~SceneGraphEdge();
	/**
	 * Copy constructor
	 * @TODO
	 */
	SceneGraphEdge(const SceneGraphEdge& b) = delete;
	/**
	 * Move constructor
	 * @TODO?
	 */
	SceneGraphEdge(const SceneGraphEdge&& b) = delete;
	/**
	 * Copy assignment operator
	 * @TODO?
	 */
	SceneGraphEdge& operator= (const SceneGraphEdge& b) = delete;
	/**
	 * Move assignment operator
	 * @TODO?
	 */
	SceneGraphEdge& operator= (const SceneGraphEdge&& b) = delete;
public:
    /////////////////////////
    // Matrix Manipulation //
    /////////////////////////
    /**
     * Resets the transformation matrix to the identity matrix
     */
	inline SceneGraphEdge& resetTransform() { localTransform = glm::mat4(1); hasChanged = true; return *this; }
    /**
     * Replaces the transformation matrix with the provided one
     * @param newTransform The new transform matrix to be used
     */
	inline SceneGraphEdge& setTransform(glm::mat4 newTransform) { localTransform = newTransform; hasChanged = true; return *this; }
    /**
     * Scales the existing transformation matrix
     * @param scale The 3-dimensional scale factor to be used
     */
	inline SceneGraphEdge& scale(glm::vec3 scale) { localTransform *= glm::scale(scale); hasChanged = true; return *this; }
    /**
     * Rotates the existing transformation matrix about an axis
     * @param axis The axis to rotate about
     * @param angleDegrees The number of degrees to rotate
     */
	inline SceneGraphEdge& rotate(glm::vec3 axis, float angleDegrees) { localTransform *= glm::rotate(angleDegrees, axis); hasChanged = true; return *this; }
    /**
     * Translates the existing transformation matrix
     * @param translation The translation to use
     */
	inline SceneGraphEdge& translate(glm::vec3 translation){ localTransform *= glm::translate(translation); hasChanged = true; return *this; }
    /**
     * Replaces the absolute translation of the transformation matrix
     * @param offset The new absolute translation
     */
	inline SceneGraphEdge& setOffset(glm::vec3 offset) {
        localTransform[3][0] = offset.x;
        localTransform[3][1] = offset.y;
        localTransform[3][2] = offset.z;
		hasChanged = true; 
		return *this;
    };
    /////////////////////////////////////
    // Convenience Matrix Manipulation //
    /////////////////////////////////////
    /**
     * Uniformly scales the existing transformation matrix
     * @param scale The scale factor to be used
     */
	inline SceneGraphEdge& scale(float scale) { return this->scale(glm::vec3(scale)); }
    /**
     * @see scale(glm::vec3)
     */
	inline SceneGraphEdge& scale(float x, float y, float z) { return this->scale(glm::vec3(x, y, z)); }
    /**
     * @see rotate(glm::vec3, float)
     */
	inline SceneGraphEdge& rotate(float axisX, float axisY, float axisZ, float angleDegrees) { return this->rotate(glm::vec3(axisX, axisY, axisZ), angleDegrees); }
    /**
     * @see translate(glm::vec3)
     */
	inline SceneGraphEdge& translate(float x, float y, float z){ return this->translate(glm::vec3(x, y, z)); }
    /**
     * @see setOffset(glm::vec3)
     */
	inline SceneGraphEdge& setOffset(float x, float y, float z){ return this->setOffset(glm::vec3(x, y, z)); }
    ///////////////
    // Accessors //
    ///////////////
    inline glm::mat4 getTransform() const { return computedGlobalTransform; }
	inline const glm::mat4 *getTransformPtr() const { return &computedGlobalTransform; }
	////////////////////////////////////
	// Advanced Scene Graph Mechanics //
	////////////////////////////////////
	/**
	 * Attaches the object in it's current offset from the edge to the scene graph
	 * @note On release this will affect the SceneGraphVertex's model matrix
	 * therefore the SceneGraphVertex should not have any existing attachments
	 */
	SceneGraphEdge& grab(const std::shared_ptr<SceneGraphVertex> &target);
	/**
	 * Drops the specified object, updating it's model matrix so that it remains in the same position it were at release
	 */
	bool release(const std::shared_ptr<SceneGraphVertex> &target);
	bool release(SceneGraphEdge& target);
private:
	inline bool expired() const { return hasChanged; }
    /**
     * Updates computedGlobalTransform in this and all child edges
     * @param parentTransform Applies this matrix before localTransform
     * @note computedGlobalTransform = parentTransform x localTransform
     */
    void propagateUpdate(const glm::mat4 &parentTransform);
    /**
     * Renders the hierarchy below this edge of the scene graph
     * @param rootTransform Model matrix applied before the scene's transformation matrix and leafs model matrix
     */
    inline void renderSceneGraph(const glm::mat4 &rootTransform) const
    {
        //Render hierarchy
        vertex->renderSceneGraph(rootTransform, computedGlobalTransform);
    }
    const std::shared_ptr<SceneGraphVertex> vertex;
    glm::mat4 localTransform;
    glm::mat4 computedGlobalTransform;
    bool hasChanged;
};

#endif //__SceneGraph_h__