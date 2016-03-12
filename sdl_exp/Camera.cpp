#include "Camera.h"

#include <stdio.h>

/*
Initialises the camera located at (1,1,1) directed at (0,0,0)
*/
Camera::Camera() 
    : Camera(glm::vec3(1, 1, 1))
{}
/*
Initialises the camera located at eye, directed at (0,0,0)
@param eye The coordinates the camera is located
*/
Camera::Camera(glm::vec3 eye)
    : Camera(eye, glm::vec3(0, 0, 0))
{}
//Initialiser list written to remove any references to member variables
//Because member variables are initialised via initaliser lists in the order they are declared in the class declaration (rather than the order of the initialiser list)
/*
Initialises the camera located at eye directed at target
@param eye The coordinates the camera is located
@param target The coordinates the camera is directed towards
*/
Camera::Camera(glm::vec3 eye, glm::vec3 target)
    : pureUp(0.0f, 1.0f, 0.0f)
    , eye(eye)
    , look(normalize(target - eye))
    , right(normalize(cross(glm::vec3(0, 1, 0), target - eye)))
    , up(normalize(cross(cross(glm::vec3(0, 1, 0), target - eye), target - eye)))
    , stabilise(true)
{
    //this->eye = eye;                                  //Eye is the location passed by user
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
/*
Default destructor
*/
Camera::~Camera(){
}
/*
Rotate look and right, yaw radians about up
Rotate look and up, pitch radians about right
@param yaw The number of radians to rotate the camera's direction about the up vector
@param pitch The number of radians to rotate the camera's direction about the right vector
*/
void Camera::turn(float yaw, float pitch){
    //Rotate everything yaw rads about up vector
    look = normalize(rotate(this->look, -yaw, this->up));
    right = normalize(rotate(this->right, -yaw, this->up));
    //Rotate everything pitch rads about right vector
    glm::vec3 look = normalize(rotate(this->look, -pitch, right));
    glm::vec3 up = normalize(rotate(this->up, -pitch, right));
    if (stabilise)
    {
        glm::vec3 right = cross(pureUp, look);    //Right is perpendicular to look and (0,1,0)[Default up]
        up = cross(right, look);
        if ((up.y < 0 && pureUp.y > 0) || (up.y > 0 && pureUp.y < 0))
        {
            up = -up;
            right = -right;
        }
        //printf("%f\n", );
        if (abs(dot(look, pureUp)) > 0.98)
            return;//Turning too close to a pole, exit early
    }
    //Commit changes
    this->look = look;
    this->right = right;
    this->up = up;
}
/*
Move eye specified distance along look
@param distance The number of units to move the camera
*/
void Camera::move(float distance){
    eye += look*distance;
}
/*
Rotate right and up, roll radians about look
@param roll The number of radians to rotate the camera's direction about the look vector
*/
void Camera::roll(float roll){
    pureUp = normalize(rotate(pureUp, roll, look));
    right = normalize(rotate(right, roll, look));
    up = normalize(rotate(up, roll, look));
}
/*
Move eye specified distance along right
@param distance The number of units to move the camera
*/
void Camera::strafe(float distance){
    eye += right*distance;
}
/*
Move eye specified distance along up
@param distance The number of units to move the camera
*/
void Camera::ascend(float distance){
    eye += up*distance;
}
/*
Returns the projection matrix 
For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_PROJECTION)
@return the projection matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
*/
glm::mat4 Camera::view(){
    return glm::lookAt(eye, eye + look, up);
}
/*
Calls gluLookAt()
For people using fixed function pipeline
@see view()
*/
void Camera::gluLookAt(){
    GL_CALL(::gluLookAt(
        eye.x, eye.y, eye.z,
        eye.x + look.x, eye.y + look.y, eye.z + look.z,
        up.x, up.y, up.z
        ));
}
/*
Returns the projection matrix from the perspective required for rendering a skybox (direction only)
For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_PROJECTION)
@return the projection matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
*/
glm::mat4 Camera::skyboxView(){
    return glm::lookAt(glm::vec3(0), look, up);
}
/*
Calls gluLookAt() from the perspective required for rendering a skybox (direction only) 
For people using fixed function pipeline
@see skyboxView()
*/
void Camera::skyboxGluLookAt(){
    GL_CALL(::gluLookAt(
        0, 0, 0,
        look.x, look.y, look.z,
        up.x, up.y, up.z
        ));
}
/*
Returns the cameras location
@return The location of the camera in world space
*/
const glm::vec3 Camera::getEye(){
    return eye;
}
/*
Returns the cameras normalized direction vector
@return The normalized direction of the camera
*/
const glm::vec3 Camera::getLook(){
    return look;
}
/*
Returns the cameras normalized up vector
@return The normalized direction the camera treats as upwards
*/
const glm::vec3 Camera::getUp(){
    return up;
}
/*
Returns the cameras normalized right vector
@return The normalized direction the camera treats as rightwards
*/
const glm::vec3 Camera::getRight(){
    return right;
}
/*
Sets whether the camera should be stabilised
When the camera is stabilised, the up vector will not rotate about the cameras direction
When the camera is not stabilsed, moving the mouse in a circular motion may cause the camera to roll
@param stabilise Whether the camera should be stabilised
*/
void Camera::setStabilise(bool stabilise){
    this->stabilise = stabilise;
}