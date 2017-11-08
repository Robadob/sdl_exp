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
    friend class SceneGraphEdge; //Accesses private renderSceneGraph
    typedef SceneGraphVertex SGVertex;
public:
    virtual ~SceneGraphVertex() = default;
    virtual glm::mat4 getModelMat() const=0;
    virtual const glm::mat4 *getModelMatPtr() const=0;
    //virtual void setModelMat(const glm::mat4 *newModelMat)=0;
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
     * Forwards propogation to each child
     * @param parentTransform Transform to be applied before local transform
     */
    void propagateUpdate(const glm::mat4 &parentTransform);
private:
    void renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform);
    std::list<SceneGraphEdge> children;
};

/**
 * This class represents an edge in a scene graph
 * These recursively combine to produce a hierarchy
 * The edge holds a sceneGraph transformation and an associated SceneGraphVertex
 */
class SceneGraphEdge
{
    typedef SceneGraphVertex SGEdge;
public:
    /**
     *
     */
    SceneGraphEdge(const std::shared_ptr<SceneGraphVertex> &vertex, const glm::mat4 &localTransform = glm::mat4(1));
    /////////////////////////
    // Matrix Manipulation //
    /////////////////////////
    /**
     * Resets the transformation matrix to the identity matrix
     */
    inline void resetTransform() { localTransform = glm::mat4(1); hasChanged = true; }
    /**
     * Replaces the transformation matrix with the provided one
     * @param newTransform The new transform matrix to be used
     */
    inline void setTransform(glm::mat4 newTransform) { localTransform = newTransform; hasChanged = true; }
    /**
     * Scales the existing transformation matrix
     * @param scale The 3-dimensional scale factor to be used
     */
    inline void scale(glm::vec3 scale) { localTransform *= glm::scale(scale); hasChanged = true; }
    /**
     * Rotates the existing transformation matrix about an axis
     * @param axis The axis to rotate about
     * @param angleDegrees The number of degrees to rotate
     */
    inline void rotate(glm::vec3 axis, float angleDegrees) { localTransform *= glm::rotate(angleDegrees, axis); hasChanged = true; }
    /**
     * Translates the existing transformation matrix
     * @param translation The translation to use
     */
    inline void translate(glm::vec3 translation){ localTransform *= glm::translate(translation); hasChanged = true; }
    /**
     * Replaces the absolute translation of the transformation matrix
     * @param offset The new absolute translation
     */
    inline void setOffset(glm::vec3 offset) {
        localTransform[3][0] = offset.x;
        localTransform[3][1] = offset.y;
        localTransform[3][2] = offset.z;
        hasChanged = true;
    };
    /////////////////////////////////////
    // Convenience Matrix Manipulation //
    /////////////////////////////////////
    /**
     * Uniformly scales the existing transformation matrix
     * @param scale The scale factor to be used
     */
    inline void scale(float scale) { this->scale(glm::vec3(scale)); }
    /**
     * @see scale(glm::vec3)
     */
    inline void scale(float x, float y, float z) { this->scale(glm::vec3(x, y, z)); }
    /**
     * @see rotate(glm::vec3, float)
     */
    inline void rotate(float axisX, float axisY, float axisZ, float angleDegrees) { this->rotate(glm::vec3(axisX, axisY, axisZ), angleDegrees); }
    /**
     * @see translate(glm::vec3)
     */
    inline void translate(float x, float y, float z){ this->translate(glm::vec3(x, y, z)); }
    /**
     * @see setOffset(glm::vec3)
     */
    inline void setOffset(float x, float y, float z){ this->setOffset(glm::vec3(x, y, z)); }
    ///////////////
    // Accessors //
    ///////////////
    inline bool expired() const { return hasChanged; }
    inline glm::mat4 getTransform() const { return computedGlobalTransform; }
    inline const glm::mat4 *getTransformPtr() const { return &computedGlobalTransform; }
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
private:
    std::shared_ptr<SceneGraphVertex> vertex;
    glm::mat4 localTransform;
    glm::mat4 computedGlobalTransform;
    bool hasChanged;
};

#endif //__SceneGraph_h__