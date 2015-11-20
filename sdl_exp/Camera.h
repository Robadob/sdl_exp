#ifndef __Camera_h__
#define __Camera_h__
/**
 * Define GLU_LOOK_AT for calls to view() to automatically call the deprecated function gluLookAt()
**/
#include "gl/glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#define USE_MATH_DEFINES
#include <math.h>
//#define GLM_FORCE_INLINE //Potentially enable this at a later date as it may slightly improve performance
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Camera
{
public:
    Camera();
    Camera(glm::vec3 eye);
    Camera(glm::vec3 eye, float yaw, float pitch);
    ~Camera();

    void turn(float thetaInc, float phiInc);
    void move(float distance);
    void strafe(float distance);
    void ascend(float distance);
    void roll(float distance);
    glm::mat4 view();
private:
    glm::vec3 eye;
    glm::vec3 look;
    glm::vec3 up;
    glm::vec3 right;
};

#endif