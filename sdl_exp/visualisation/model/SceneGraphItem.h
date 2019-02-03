#ifndef __SceneGraphItem_h__
#define __SceneGraphItem_h__
#include <glm/mat4x4.hpp>
#include <memory>
#include <list>
#include <glm/gtx/transform.hpp>
#include "TransformationMatrix.h"

class SceneGraphItem;
class SceneGraphJoint;
typedef SceneGraphItem SGItem;
/**
 * This class represents a vertex in a scene graph
 * These recursively combine to produce a hierarchy
 */
class SceneGraphItem : public std::enable_shared_from_this<SceneGraphItem>
{
protected:
    SceneGraphItem();
    /**
     * Copy constructor
     * @TODO: Decide how to handle children/parents on copy
     */
    SceneGraphItem(const SceneGraphItem& b) = delete;
    /**
     * Move constructor
     * Transfers values and children/parent relationships
     */
    SceneGraphItem(SceneGraphItem&& b);
    /**
     * Forwards propogation to each child
     * @param sceneTransform Transform to be applied before local transform of the attachment
     */
    void propagateUpdate(const glm::mat4 &sceneTransform);
public:
    /**
     * Copy assignment operator
     * @TODO: Decide how to handle children/parents on copy
     */
    SceneGraphItem& operator= (const SceneGraphItem& b) = delete;
    /**
     * Move assignment operator
     * Transfers values and children/parent relationships
     */
    SceneGraphItem& operator= (SceneGraphItem&& b);
    virtual ~SceneGraphItem() = default;
    glm::matT getSceneMat() const { return sceneMat; };
    const glm::matT *getSceneMatPtr() const{ return &sceneMat; }
    const glm::matT &getSceneMatRef() const{ return sceneMat; }
    glm::matT getLocalMat() const { return localMat; };
    const glm::matT *getLocalMatPtr() const{ return &localMat; }
    const glm::matT &getLocalMatRef() const{ return localMat; }
    glm::mat4 getModelMat() const { return computedModelMat; };
    const glm::mat4 *getModelMatPtr() const{ return &computedModelMat; }
    const glm::mat4 &getModelMatRef() const{ return computedModelMat; }
    ///////////////////////
    // Scene Graph Usage //
    ///////////////////////
    /**
     * Renders the current object applying transform before the internal model matrix
     * @param transform Transformation matrix to be applied before the model matrix
     * @note Matrix order: projection x view x transform x model
     * @note You are required to apply the modelMatrix transform yourself, as this method may be called externally
     * @note This fn cannot be made constant, as Materials change to track active.
     */
    virtual void render(const unsigned int &shaderIndex = UINT_MAX, const glm::mat4 &transform = glm::mat4(1)) = 0;
    inline void render(const glm::mat4 &transform)
    {
        render(UINT_MAX, transform);
    }
    /**
     * Renders the scene graph hierarchy below the current item
     * @param transform Transformation matrix to be applied before the scene graph and model matrix
     * @note Matrix order: projection x view x transform x scene x model x internal
     */
    inline void renderSceneGraph(const unsigned int &shaderIndex = UINT_MAX, const glm::mat4 &transform = glm::mat4(1))
    {
        renderSceneGraph(shaderIndex, transform, glm::mat4(1));
    }
    inline void renderSceneGraph(const glm::mat4 &transform)
    {
        renderSceneGraph(UINT_MAX, transform, glm::mat4(1));
    }
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
     * @note This plan might be flawed if a model requires a dynamic attachment point
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
    bool attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, const glm::mat4 &attachmentTransform = glm::mat4(1));
    bool attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, glm::vec3 parentAttachOffset, glm::vec3 childAttachOffset = glm::vec3(0));
    bool attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, unsigned int parentAttachOffsetIndex, unsigned int childAttachOffsetIndex = 0);
    //SceneGraphJoint versions
    //Implemented in SceneGraphJoint.cpp
    bool attach(const std::shared_ptr<SceneGraphJoint> &child, const std::string &reference, glm::vec3 parentAttachOffset);
    bool attach(const std::shared_ptr<SceneGraphJoint> &child, const std::string &reference, unsigned int parentAttachOffsetIndex = 0);

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
    std::shared_ptr<SceneGraphItem> getAttachment(const std::string &reference);    
    /**
    * Confirms whether any children (direct or recursively found) already exists of the provided instance 'child'
    * @param child The instance to check for
    * @return True if the provided instance is already attached
    * @note Does not confirm whether children of the instance already exist
    */
    bool hasChildAttachment(const std::shared_ptr<const SceneGraphItem> &child) const;
    /////////////////////////////////
    // Scene Graph Parent Tracking //
    // --prevents cycle creation-- //
    /////////////////////////////////
    /**
     * Recursively checks for the presence of paramter 'parent' among parents
     * @param parent to check for
     * @return True if parent found, else false
     */
    bool hasParentAttachment(const std::shared_ptr<const SceneGraphItem> &parent) const;

    bool getSceneGraphVisible() const { return sceneGraphVisible; }
private:
    /**
     * Internally used when adding/removing children to prevent cycles
     * @param parent The new parent
     * @note An instance can have the same parent multiple times in which case it is inserted multiple times into the list
     */
    void addParent(const std::shared_ptr<SceneGraphItem> &parent);
    /**
     * Internally used when adding/removing children to prevent cycles
     * @param parent The leaving parent
     * @note An instance can have the same parent multiple times in which case it is removed ONCE
     * @note Will throw an assertion if the parent is not found to be removed
     */
    void removeParent(const std::shared_ptr<const SceneGraphItem> &parent);
#ifdef _DEBUG
    /**
     * Recursively returns a list of child
     * @note Convert to raw pointer because cheaper than copying shared pointers
     */
    std::list<std::shared_ptr<SceneGraphItem>> getChildPtrs() const;
#endif
    /**
     * This is used for comparing parental heritage only
     * These pointers shouldn't be used for anything else
     */
    std::list<const std::weak_ptr<SceneGraphItem>> parents;
    /**
     * Called by parent SceneGraphEdge's, passing the computedGlobalTransform as sceneTransform
     */
    void renderSceneGraph(const unsigned int &shaderIndex, const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform);
    /**
     * Struct for holding information necessary to render and remove/access children
     */
    struct AttachmentDetail
    {
        std::string reference;
        std::shared_ptr<SceneGraphItem> child;
        //glm::vec3 parentOffset;
        //glm::vec3 childOffset;
        //glm::vec3 inverseScale;//Used to ignore parents scale transform
        glm::mat4 attachmentTransform;
        glm::mat4 computedTransformMat;
        static glm::vec3 getScale(const glm::mat4 &m)
        {
            return glm::vec3(
                length(glm::vec3(m[0][0], m[1][0], m[2][0])),
                length(glm::vec3(m[0][1], m[1][1], m[2][1])),
                length(glm::vec3(m[0][2], m[1][2], m[2][2]))
                );
        }
        static glm::mat4 getMat(const glm::vec3 &parentOffset, const glm::vec3 &childOffset, const glm::vec3 &inverseScale)
        {
            return glm::scale(inverseScale) * glm::translate(parentOffset - childOffset);
        }
        void setComputedTransformMat(const glm::mat4 &sceneParentTransform)
        {
            computedTransformMat = sceneParentTransform * attachmentTransform;
        }
    };
    std::list<AttachmentDetail> children;
    /**
     * Marks whether the subscene graph requires recomputing transform matrices
     */
    bool expired;
    /**
     * This holds model matrix transforms which should be applied to scene children
     * modelMat transforms are NOT applied to children.
     * sceneModelMat = sceneMat * modelMat should be passed as modelMat to shaders
     * There is currently no mechanism for caching sceneModelMat value
     */
    glm::matT sceneMat;
    /**
     * This holds model matrix transforms which should be applied only to this item, and children
     * localMat transforms are NOT applied to children.
     * sceneModelMat = sceneMat * modelMat should be passed as modelMat to shaders
     * There is currently no mechanism for caching sceneModelMat value
     */
    glm::matT localMat;
    /**
     * An internal matrix to be used by the subclass
     * An example use of this would be if the scale of the model is configured by the loader
     */
    glm::matT internalMat;
    /**
     * modelMat = sceneMat * localMat * internalMat
     */
    glm::mat4 computedModelMat;
    void recomputeModelMat() { computedModelMat = sceneMat * localMat * internalMat; }
    /**
     * Denotes whether scene graph is rendered or hidden
     */
    bool sceneGraphVisible;
protected:
    /**
     * Used by subclasses to updated the model matrix
     * This also sets the flag 'expired' to true so that scene graphs can be rebuilt
     */
    inline void setSceneMat(const glm::mat4 &m) { sceneMat = m; expired = true; recomputeModelMat(); }
    inline void setLocalMat(const glm::mat4 &m) { localMat = m; recomputeModelMat(); }
    inline void setInternalMat(const glm::mat4 &m) { internalMat = m; recomputeModelMat(); }
    /**
     * These scene and local matrix methods can be exposed as required by subclasses
     */
    //Scene Space/Mat Transforms, these affect children of the entity
    glm::vec3 getLocation() const { return sceneMat[3]; };
    const glm::vec3 *getLocationPtr() const { return (glm::vec3*)&sceneMat[3]; }
    void setSceneTranslation(const glm::vec3 &loc){ sceneMat.translateA(loc); expired = true;  recomputeModelMat(); }
    void translateScene(const glm::vec3 &offset){ sceneMat.translateR(offset); expired = true; recomputeModelMat(); }
    void setSceneRotation(const float &angleRads, const glm::vec3 &axis) { sceneMat.rotateE(angleRads, axis); expired = true; recomputeModelMat(); }

    //Local Space/Mat Transforms, these DO NOT affect children of the entity
    void setLocalTranslation(const glm::vec3 &loc){ localMat.translateA(loc); recomputeModelMat(); }
    void translateLocal(const glm::vec3 &offset){ localMat.translateR(offset); recomputeModelMat();  }
    void setLocalRotation(const float &angleRads, const glm::vec3 &axis) { localMat.rotateE(angleRads, axis); recomputeModelMat(); }
    /**
     * Hides or displays scene graph when renderSceneGraph() is called.
     */
    void setSceneGraphVisible(const bool &isVisible) { sceneGraphVisible = isVisible; };
};

#endif //__SceneGraphItem_h__