#include "Camera.h"
#include <stdio.h>

Camera::Camera() : 
    Camera(glm::vec3(1, 1, 1))
{}
Camera::Camera(glm::vec3 eye)
    : Camera(eye, glm::vec3(0, 0, 0))
{}
//Initialiser list written to remove any references to member variables
//Because member variables are initialised via initaliser lists in the order they are declared in the class declaration
Camera::Camera(glm::vec3 eye, glm::vec3 target)
    : eye(eye)
    , look(normalize(target - eye))
    , right(normalize(cross(glm::vec3(0, 1, 0), target - eye)))
    , up(normalize(cross(cross(glm::vec3(0, 1, 0), target - eye), target - eye)))
{
    //this->eye = eye;                                    //Eye is the location passed by user
    //this->look = target - eye;                        //Look is the direction from eye to target
    //this->right = cross(glm::vec3(0, 1, 0), look);    //Right is perpendicular to look and (0,1,0)[Default up]
    //this->up = cross(right, look);                    //Up is perpendicular to right and look
    //Make sure Up is not upside down
    if (up.y < 0)
    {
        up = -up;
        right = -right;
    }
}

Camera::~Camera(){
}
/*
Rotate look and right, yaw radians about up
Rotate look and up, pitch radians about right
*/
void Camera::turn(float yaw, float pitch){
    //Rotate everything yaw rads about up vector
    look = normalize(rotate(this->look, -yaw, this->up));
    right = normalize(rotate(this->right, -yaw, this->up));
    //Rotate everything pitch rads about right vector
    look = normalize(rotate(this->look, -pitch, this->right));
    up = normalize(rotate(this->up, -pitch, this->right));
}
/*
Move eye specified distance along look
*/
void Camera::move(float distance){
    //
    eye += look*distance;
}
/*
Rotate right and up, roll radians about look
*/
void Camera::roll(float roll)
{
    right = normalize(rotate(right, roll, look));
    up = normalize(rotate(up, roll, look));
}
/*
Move eye specified distance along right
*/
void Camera::strafe(float distance){
    eye += right*distance;
}
/*
Move eye specified distance along up
*/
void Camera::ascend(float distance){
    eye += up*distance;
}
/*
Returns the projection matrix for use with shader uniforms or glLoadMatrixf() [after calling glMatrixMode(GL_PROJECTION)]
*/
glm::mat4 Camera::view(){
    return glm::lookAt(eye, eye + look, up);
}
/*
Calls gluLookAt() for people using fixed function pipeline
*/
void Camera::gluLookAt()
{
    ::gluLookAt(
        eye.x, eye.y, eye.z,
        eye.x + look.x, eye.y + look.y, eye.z + look.z,
        up.x, up.y, up.z
        );
}
/*
Returns the cameras location
*/
const glm::vec3 Camera::getEye()
{
    return eye;
}
/*
Returns the cameras normalized direction
*/
const glm::vec3 Camera::getLook()
{
    return look;
}