#include "Camera.h"

#include <glm/gtx/rotate_vector.hpp>

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
    this->updateViews();
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
    this->look = normalize(rotate(this->look, -yaw, this->up));
    this->right = normalize(rotate(this->right, -yaw, this->up));
    //Rotate everything pitch rads about right vector
    glm::vec3 look = normalize(rotate(this->look, -pitch, this->right));
    glm::vec3 up = normalize(rotate(this->up, -pitch, this->right));
    glm::vec3 right = this->right;
    if (stabilise)
    {    
        //Right is perpendicular to look and (0,1,0)[Default up]
        right = cross(this->pureUp, look);
        //Stabilised up is perpendicular to right and look
        up = cross(right, look);
        //Flip up and right if backwards
        if ((up.y < 0 && this->pureUp.y > 0) || (up.y > 0 && this->pureUp.y < 0))
        {
            up = -up;
            right = -right;
        }
        //If the look vector gets too close to a pole, exit early to stop infinite rotation bug
        if (abs(dot(look, this->pureUp)) > 0.98)
            return;
    }
    //Commit changes
    this->look = look;
    this->right = right;
    this->up = up;
    this->updateViews();
}
/*
Move eye specified distance along look
@param distance The number of units to move the camera
*/
void Camera::move(float distance){
    eye += look*distance;
    this->updateViews();
}
/*
Rotate right and up, roll radians about look
@param roll The number of radians to rotate the camera's direction about the look vector
*/
void Camera::roll(float roll){
    pureUp = normalize(rotate(pureUp, roll, look));
    right = normalize(rotate(right, roll, look));
    up = normalize(rotate(up, roll, look));
    this->updateViews();
}
/*
Move eye specified distance along right
@param distance The number of units to move the camera
*/
void Camera::strafe(float distance){
    eye += right*distance;
    this->updateViews();
}
/*
Move eye specified distance along up
@param distance The number of units to move the camera
*/
void Camera::ascend(float distance){
    eye += up*distance;
    this->updateViews();
}
/*
Updates the view and skyboxView matrices
Called whenever any internal camera variables are updated
*/
void Camera::updateViews(){
    viewMat = glm::lookAt(eye, eye + look, up);
    skyboxViewMat = glm::lookAt(glm::vec3(0), look, up);
}
/*
Returns the projection matrix 
For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
@return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
*/
glm::mat4 Camera::view() const{
    return viewMat;
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
For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
@return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
*/
glm::mat4 Camera::skyboxView() const{
    return skyboxViewMat;
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
glm::vec3 Camera::getEye() const{
    return eye;
}
/*
Returns the cameras normalized direction vector
@return The normalized direction of the camera
*/
glm::vec3 Camera::getLook() const{
    return look;
}
/*
Returns the cameras normalized up vector
@return The normalized direction the camera treats as upwards
*/
glm::vec3 Camera::getUp() const{
    return up;
}
/*
Returns the value of pureUp
This value is used by the stabilisation to prevent the camera rolling unintentionally
@return The normalized direction the camera treats as the true up
*/
glm::vec3 Camera::getPureUp() const{
    return pureUp;
}
/*
Returns the cameras normalized right vector
@return The normalized direction the camera treats as rightwards
*/
glm::vec3 Camera::getRight() const{
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
/*
Returns a constant pointer to the cameras modelview matrix
This pointer can be used to continuously track the modelview matrix
@return A pointer to the modelview matrix
*/
const glm::mat4 *Camera::getViewMatPtr() const{
    return &viewMat;
}
/*
Returns a constant pointer to the cameras skybox modelview matrix
This pointer can be used to continuously track the skybox modelview matrix
@return A pointer to the modelview matrix
*/
const glm::mat4 *Camera::getSkyboxViewMatPtr() const{
    return &skyboxViewMat;
}