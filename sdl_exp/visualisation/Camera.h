#ifndef __Camera_h__
#define __Camera_h__

#include "GLcheck.h"

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();
    Camera(glm::vec3 eye); 
    Camera(glm::vec3 eye, glm::vec3 target);
    ~Camera();

    void turn(float thetaInc, float phiInc);
    void move(float distance);
    void strafe(float distance);
    void ascend(float distance);
    void roll(float distance);
    void setStabilise(bool stabilise);
    glm::mat4 view() const; 
    void gluLookAt();
    glm::mat4 skyboxView() const;
    void skyboxGluLookAt() const;
    glm::vec3 getEye() const;
    glm::vec3 getLook() const;
    glm::vec3 getUp() const;
    glm::vec3 getPureUp() const;
    glm::vec3 getRight() const;
    const glm::mat4 *Camera::getViewMatPtr() const;
    const glm::mat4 *Camera::getSkyboxViewMatPtr() const;
private:
    void updateViews();
    //ModelView matrix
    glm::mat4 viewMat;
    //Model view matrix without camera position taken into consideration
    glm::mat4 skyboxViewMat;
    //Up vector used for stabilisation, only rotated when roll is called
    glm::vec3 pureUp;
    //Eyelocation
    glm::vec3 eye;
    //3 perpendicular vectors which represent the cameras direction and orientation
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
    bool stabilise;
};

#endif //ifndef __Camera_h__