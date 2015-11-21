#ifndef __Camera_h__
#define __Camera_h__

#include "gl/glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#define USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Camera
{
public:
    Camera();
    Camera(glm::vec3 eye); 
    Camera(glm::vec3 eye, glm::vec3 target);
    Camera(glm::vec3 eye, float yaw, float pitch);
    ~Camera();

    void turn(float thetaInc, float phiInc);
    void move(float distance);
    void strafe(float distance);
    void ascend(float distance);
    void roll(float distance);
    glm::mat4 view(); 
    void gluLookAt();
    const glm::vec3 getEye();
    const glm::vec3 getLook();
private:
    glm::vec3 eye;
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
};

#endif