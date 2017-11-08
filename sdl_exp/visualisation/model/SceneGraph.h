#ifndef __SceneGraph_h__
#define __SceneGraph_h__
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <list>

class SceneGraphEdge;
/**
 * This class represents a vertex in a scene graph
 * These recursively combine to produce a hierarchy 
 */
class SceneGraphVertex
{
    friend class SceneGraphEdge; //Accesses private renderSceneGraph(const glm::mat4 &, const glm::mat4 &)
    typedef SceneGraphVertex SGVertex;
public:
	SceneGraphVertex();
	/**
	 * Copy constructor
	 * @TODO
	 */
	SceneGraphVertex(const SceneGraphVertex& b) = delete;
	/**
	 * Move constructor
	 * @TODO?
	 */
	SceneGraphVertex(const SceneGraphVertex&& b) = delete;
	/**
	 * Copy assignment operator
	 * @TODO?
	 */
	SceneGraphVertex& operator= (const SceneGraphVertex& b) = delete;
	/**
	 * Move assignment operator
	 * @TODO?
	 */
	SceneGraphVertex& operator= (const SceneGraphVertex&& b) = delete;
    virtual ~SceneGraphVertex() = default;
    virtual glm::mat4 getModelMat() const=0;
    virtual const glm::mat4 *getModelMatPtr() const=0;
    virtual void setModelMat(const glm::mat4 &newModelMat)=0;
    /**
     * Renders the current object applying transform before the internal model matrix
     * @param transform Transformation matrix to be applied before the model matrix
     * @note Matrix order: projection x view x transform x model
     */
    virtual void render(const glm::mat4 &transform)=0;
    /**
     * Renders the scene graph hierarchy below the current item
     * @param transform Transformation matrix to be applied before the scene graph and model matrix
     * @note Matrix order: projection x view x transform x scene x model
     */
    inline void renderSceneGraph(const glm::mat4 &transform)
    {
        renderSceneGraph(transform, glm::mat4(1));
    }
	/**
	 * Attaches a SceneGraphVertex to this models sub-graph with the given attachment transform
	 */
	SceneGraphEdge &addChild(const std::shared_ptr<SceneGraphVertex> &child, const glm::mat4 &attachmentTransform = glm::mat4(1));
	/**
	 * Removes the specified edge from this model's sub graph
	 */
	void removeChild(const SceneGraphEdge &edge);
	/**
	 * Returns a pointer to the first direct child of this model which shares the given pointer, else nullptr
	 */
	const SceneGraphEdge *getChildEdge(const std::shared_ptr<SceneGraphVertex> &child);
    /**
     * Forwards propogation to each child
     * @param parentTransform Transform to be applied before local transform
     */
    void propagateUpdate(const glm::mat4 &parentTransform);
	/**
	 * Tracks the number of SceneGraphEdge's this SceneGraphVertex is contained within
	 */
	inline unsigned int attachmentCount() const { return attachments; }
private:
    void renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform);
	void incrementAttachments() { attachments++; };
	/**
	 * Safety check to ensure attachments doesn't loop
	 */
	void decrementAttachments() { if (attachments)attachments--; };
	unsigned int attachments;
    std::list<SceneGraphEdge> children;
};

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