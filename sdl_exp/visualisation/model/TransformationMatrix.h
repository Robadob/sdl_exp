#ifndef __TransformationMatrix_h__
#define __TransformationMatrix_h__
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <cstdio>
class TransformationMatrix;
namespace glm
{
    typedef TransformationMatrix matT;
}
//Provides access to
class TransformationMatrix : public glm::mat4
{
public:
    TransformationMatrix()
        : glm::mat4(1)//Identity matrix
    { }
    TransformationMatrix(const glm::mat4 &m)
        : glm::mat4(m)
    { }
    /**
     * Translation Accessors
     */
    glm::vec3 translation() const
    {
        return (*this)[3];//Implictly drop 4th component
    }
    void translateA(const glm::vec3 &loc)
    {
        glm::mat4 &m = *this;
        m[3][0] = loc[0];
        m[3][1] = loc[1];
        m[3][2] = loc[2];
    }
    void translateR(const glm::vec3 &offset)
    {
        glm::mat4 &m = *this;
        m[3][0] += offset[0];
        m[3][1] += offset[1];
        m[3][2] += offset[2];
    }
    /**
     * Scale Accessors
     */
    glm::vec3 scale() const
    {
        const glm::mat4 &m = *this;
        return glm::vec3(
            glm::length(glm::vec3(m[0][0], m[1][0], m[2][0])),
            glm::length(glm::vec3(m[0][1], m[1][1], m[2][1])),
            glm::length(glm::vec3(m[0][2], m[1][2], m[2][2]))
            );
    }
    /**
     * Inclusive, affects translation
     */
    void scaleRI(const glm::vec3 &scale)
    {
        this->operator*=(glm::scale(scale));
    }
    void uniformScaleRI(const float &scale)
    {
        scaleRI(glm::vec3(scale));
    }
    void scaleAI(const glm::vec3 &scale)
    {
        resetScaleI();
        this->operator*=(glm::scale(scale));
    }
    void uniformScaleAI(const float &scale)
    {
        scaleAI(glm::vec3(scale));
    }
    void resetScaleI()
    {
        this->operator*=(glm::scale(1.0f / scale()));
    }
    /**
    * Exclusive, does not affect translation
    */
    void scaleRE(const glm::vec3 &scale)
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        //Scale
        scaleRI(scale);
        //Reapply translation
        (*this)[3] = location;
    }
    void uniformScaleRE(const float &scale)
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        //Scale
        uniformScaleRI(scale);
        //Reapply translation
        (*this)[3] = location;
    }
    void scaleAE(const glm::vec3 &scale)
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        //Scale
        scaleAI(scale);
        //Reapply translation
        (*this)[3] = location;
    }
    void uniformScaleAE(const float &scale)
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        //Scale
        uniformScaleAI(scale);
        //Reapply translation
        (*this)[3] = location;
    }
    void resetScaleRE()
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        //Scale
        resetScaleI();
        //Reapply translation
        (*this)[3] = location;
    }
    /**
     * Rotation Accessors
     */
    //Performs an inclusive rotation
    //Rotating the matrix affecting any translations in the process
    void rotateI(const float &angleRads, const glm::vec3 &axis)
    {
        this->operator*=(glm::rotate(angleRads, axis));
    }
    //Performs an exclusive rotation
    //Rotating the matrix without affecting it's translation
    void rotateE(const float &angleRads, const glm::vec3 &axis)
    {
        //Get current translation
        glm::vec4 location = (*this)[3];
        (*this)[3] = glm::vec4(0, 0, 0, 1);//Needed?
        //Apply rotation
        (*this) *= glm::rotate(angleRads, axis);
        //Reapply translation
        (*this)[3] = location;
    }
    glm::quat orientationQuaternion() const
    {
        glm::vec3 scale, translation, skew;
        glm::quat orientation;
        glm::vec4 perspective;

        if(glm::decompose(*this, scale, orientation, translation, skew, perspective))
            return orientation;
        fprintf(stderr, "GLM failed to decompose matrix to extract orientation!\n");
        return glm::quat();
    }
    /**
     * x=pitch
     * y=yaw
     * z=roll
     */
    glm::vec3 orientationEuler() const
    {
        return glm::eulerAngles(orientationQuaternion());
    }
};
#endif //__TransformationMatrix_h__