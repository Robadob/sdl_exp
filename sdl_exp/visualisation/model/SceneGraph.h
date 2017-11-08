#ifndef __SceneGraph_h__
#define __SceneGraph_h__
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

class SceneGraphEdge
{
    /**
     *
     */
    SceneGraphEdge()
        : localTransform(1)
        , computedGlobalTransform(1)
        , hasChanged(false)
    {
        
    }
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
    inline void updateTransform(const glm::mat4 &parentTransform) { computedGlobalTransform = parentTransform * localTransform; hasChanged = false; }
private:
    glm::mat4 localTransform;
    glm::mat4 computedGlobalTransform;
    bool hasChanged;
};
/**
 * This class represents a vertex in a scene graph
 * These recursively combine to produce a hierarchy 
 */
class SceneGraphVertex
{
    typedef SceneGraphVertex SGVertex;

    virtual glm::mat4 getModelMat() const;
    virtual const glm::mat4 *getModelMatPtr() const;
    virtual void setModelMat(glm::mat4 newModelMat) const;

};
class Renderable : public SceneGraphVertex
{
    
};

#endif __SceneGraph_h__