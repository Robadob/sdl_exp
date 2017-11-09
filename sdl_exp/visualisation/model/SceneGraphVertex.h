#ifndef __SceneGraphVertex_h__
#define __SceneGraphVertex_h__
#include <glm/mat4x4.hpp>
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
	virtual glm::mat4 getModelMat() const = 0;
	virtual const glm::mat4 *getModelMatPtr() const = 0;
	virtual void setModelMat(const glm::mat4 &newModelMat) = 0;
	/**
	 * Renders the current object applying transform before the internal model matrix
	 * @param transform Transformation matrix to be applied before the model matrix
	 * @note Matrix order: projection x view x transform x model
	 */
	virtual void render(const glm::mat4 &transform) = 0;
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
	/**
	 * Called by parent SceneGraphEdge's, passing the computedGlobalTransform as sceneTransform
	 */
	void renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform);
	/**
	 * Called by SceneGraphEdge's constructor
	 */
	void incrementAttachments() { attachments++; };
	/**
	 * Called by SceneGraphEdge's destructor
	 * Includes safety check to ensure attachments doesn't loop
	 */
	void decrementAttachments() { assert(attachments);attachments--; };
	unsigned int attachments;
	std::list<SceneGraphEdge> children;
};

#endif //__SceneGraphVertex_h__