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
    void skyboxGluLookAt();
    glm::vec3 getEye() const;
    glm::vec3 getLook() const;
    glm::vec3 getUp() const;
    glm::vec3 getPureUp() const;
    glm::vec3 getRight() const;
    const glm::mat4 *Camera::getViewMatPtr() const;
    const glm::mat4 *Camera::getSkyboxViewMatPtr() const;
private:
    void updateViews();
    glm::mat4 viewMat;
    glm::mat4 skyboxViewMat;
    glm::vec3 pureUp;
    glm::vec3 eye;
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
    bool stabilise;
};

#endif //ifndef __Camera_h__