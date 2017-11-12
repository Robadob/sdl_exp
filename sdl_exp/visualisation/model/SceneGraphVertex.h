#ifndef __SceneGraphVertex_h__
#define __SceneGraphVertex_h__
#include <glm/mat4x4.hpp>
#include <memory>
#include <list>
#include <glm/gtx/transform.hpp>

class SceneGraphEdge;
/**
 * This class represents a vertex in a scene graph
 * These recursively combine to produce a hierarchy
 */
class SceneGraphVertex : public std::enable_shared_from_this<SceneGraphVertex>
{
	friend class SceneGraphEdge; //Accesses private renderSceneGraph(const glm::mat4 &, const glm::mat4 &)
	typedef SceneGraphVertex SGVertex;
public:
	SceneGraphVertex();
	/**
	 * Copy constructor
	 * @TODO: Decide how to handle children/parents on copy
	 */
	SceneGraphVertex(const SceneGraphVertex& b) = delete;
	/**
	 * Move constructor
	 * Transfers values and children/parent relationships
	 */
	SceneGraphVertex(SceneGraphVertex&& b);
	/**
	 * Copy assignment operator
	 * @TODO: Decide how to handle children/parents on copy
	 */
	SceneGraphVertex& operator= (const SceneGraphVertex& b) = delete;
	/**
	 * Move assignment operator
	 * Transfers values and children/parent relationships
	 */
	SceneGraphVertex& operator= (SceneGraphVertex&& b);
	virtual ~SceneGraphVertex() = default;
	glm::mat4 getModelMat() const { return modelMat; };
	const glm::mat4 *getModelMatPtr() const{ return &modelMat; }
	const glm::mat4 &getModelMatRef() const{ return modelMat; }
	///////////////////////
	// Scene Graph Usage //
	///////////////////////
	/**
	 * Renders the current object applying transform before the internal model matrix
	 * @param transform Transformation matrix to be applied before the model matrix
	 * @note Matrix order: projection x view x transform x model
	 * @note You are required to apply the modelMatrix transform yourself, as this method may be called externally
	 */
	virtual void render(const glm::mat4 &transform) = 0;
	/**
	 * Renders the scene graph hierarchy below the current item
	 * @param transform Transformation matrix to be applied before the scene graph and model matrix
	 * @note Matrix order: projection x view x transform x scene x model
	 */
	inline void renderSceneGraph(const glm::mat4 &transform=glm::mat4(1))
	{
		renderSceneGraph(transform, glm::mat4(1));
	}

	/**
	* Forwards propogation to each child
	* @param sceneTransform Transform to be applied before local transform of the attachment
	*/
	void propagateUpdate(const glm::mat4 &sceneTransform);
	/////////////////////////
	// Attachment Offsets //
	////////////////////////
	/**
	 * This method is intended to return the indexed attachment offset (in model space)
	 * If the index doesn't exist glm::vec3(0) should be returned
	 * index 0 returns the offset where this entity attaches to a parent entity as a child
	 * index 1+ should return the offset for any attachment points where this entity attaches to a child entity as a parent
	 * @param index The index of the attachment to return
	 * @return The offset from the model space origin of the given attachment point, if index is invalid the origin is returned
	 * @see unsigned int getAttachmentOffsetCount()
	 * @note It is not intended for attachment offset methods are extended by generic model loader types, as model
	 *       formats are unlikely to provide such data. Instead it is expected that one may extend such a class to
	 *       produce a specific class for the intended object which manually configures such offsets
	 */
	virtual glm::vec3 getAttachmentOffset(unsigned int index) const { return glm::vec3(0); }
	/**
	 * Returns the number of valid attachments
	 * @see glm::vec3 getAttachmentOffset(unsigned int index)
	 */
	virtual unsigned int getAttachmentOffsetCount() const { return 0; }
	///////////////////////////////////////
	// Scene Graph Attachment Management //
	///////////////////////////////////////
	bool attach(const std::shared_ptr<SceneGraphVertex> &child, const std::string &reference, unsigned int parentAttachOffsetIndex, unsigned int childAttachOffsetIndex = 0);
	bool attach(const std::shared_ptr<SceneGraphVertex> &child, const std::string &reference, glm::vec3 parentAttachOffset, glm::vec3 childAttachOffset = glm::vec3(0));
	/**
	 * Detatches the first child which shares the reference
	 * First checks direct children
	 * After which depth-first recursion is applied
	 * @param reference The attachment reference to detatch
	 * @return True if an attachment with the same reference was found (and detatched), else false
	 */
	bool detach(const std::string &reference);
	/**
	 * Returns the first child which shares the reference
	 * First checks direct children
	 * After which depth-first recursion is applied
	 * @param reference The attachment reference to return
	 * @return The found attachment, else empty shared_ptr
	 */
	std::shared_ptr<SceneGraphVertex> getAttachment(const std::string &reference);	
	/**
	* Confirms whether any children (direct or recursively found) already exists of the provided instance 'child'
	* @param child The instance to check for
	* @return True if the provided instance is already attached
	* @note Does not confirm whether children of the instance already exist
	*/
	bool hasChildAttachment(const std::shared_ptr<const SceneGraphVertex> &child) const;
	/////////////////////////////////
	// Scene Graph Parent Tracking //
	// --prevents cycle creation-- //
	/////////////////////////////////
	/**
	 * Recursively checks for the presence of paramter 'parent' among parents
	 * @param parent to check for
	 * @return True if parent found, else false
	 */
	bool hasParentAttachment(const std::shared_ptr<const SceneGraphVertex> &parent) const;
private:
	/**
	 * Internally used when adding/removing children to prevent cycles
	 * @param parent The new parent
	 * @note An instance can have the same parent multiple times in which case it is inserted multiple times into the list
	 */
	void addParent(const std::shared_ptr<SceneGraphVertex> &parent);
	/**
	 * Internally used when adding/removing children to prevent cycles
	 * @param parent The leaving parent
	 * @note An instance can have the same parent multiple times in which case it is removed ONCE
	 * @note Will throw an assertion if the parent is not found to be removed
	 */
	void removeParent(const std::shared_ptr<const SceneGraphVertex> &parent);
#ifdef _DEBUG
	/**
	 * Recursively returns a list of child
	 * @note Convert to raw pointer because cheaper than copying shared pointers
	 */
	std::list<std::shared_ptr<SceneGraphVertex>> getChildPtrs() const;
#endif
	/**
	 * This is used for comparing parental heritage only
	 * These pointers shouldn't be used for anything else
	 */
	std::list<const std::weak_ptr<SceneGraphVertex>> parents;
	/**
	 * Called by parent SceneGraphEdge's, passing the computedGlobalTransform as sceneTransform
	 */
	void renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform);
	/**
	 * Struct for holding information necessary to render and remove/access children
	 */
	struct AttachmentDetail
	{
		std::string reference;
		std::shared_ptr<SceneGraphVertex> child;
		glm::vec3 parentOffset;
		glm::vec3 childOffset;
		glm::mat4 computedTransformMat;
		void setComputedTransformMat(const glm::mat4 &sceneParentTransform)
		{
			computedTransformMat = sceneParentTransform * glm::translate(parentOffset - childOffset);
		}
	};
	std::list<AttachmentDetail> children;
	/**
	 * Marks whether the subscene graph requires recomputing transform matrices
	 */
	bool expired;
	glm::mat4 modelMat;
protected:
	/**
	 * Used by subclasses to updated the model matrix
	 * This also sets the flag 'expired' to true so that scene graphs can be rebuilt
	 */
	inline void setModelMat(const glm::mat4 &m) { modelMat = m; expired = true; }
};

#endif //__SceneGraphVertex_h__