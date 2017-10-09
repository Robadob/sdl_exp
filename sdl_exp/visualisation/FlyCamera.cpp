#include "FlyCamera.h"

#include "util/GLcheck.h"
#include <glm/gtx/rotate_vector.hpp>

FlyCamera::FlyCamera()
    : FlyCamera(glm::vec3(1, 1, 1))
{}
FlyCamera::FlyCamera(const glm::vec3 eye)
    : FlyCamera(eye, glm::vec3(0, 0, 0))
{}
//Initialiser list written to remove any references to member variables
//Because member variables are initialised via initaliser lists in the order they are declared in the class declaration (rather than the order of the initialiser list)
FlyCamera::FlyCamera(const glm::vec3 eye, const glm::vec3 target)
    : Camera(eye)
    , pureUp(0.0f, 1.0f, 0.0f)
    , look(normalize(target - eye))
    , right(normalize(cross(target - eye, glm::vec3(0, 1, 0))))
    , up(normalize(cross(cross(target - eye, glm::vec3(0, 1, 0)), target - eye)))
    , stabilise(true)
{
    //this->eye = eye;                                  //Eye is the location passed by user
    //this->look = target - eye;                        //Look is the direction from eye to target
    //this->right = cross(look, pureUp);                //Right is perpendicular to look and pureUp
    //this->up = cross(right, look);                    //Up is perpendicular to right and look

    this->updateViews();
}
FlyCamera::~FlyCamera(){
}
void FlyCamera::turn(float yaw, float pitch){
    //Rotate everything yaw rads about up vector
    this->look = rotate(this->look, -yaw, this->up);
    this->right = rotate(this->right, -yaw, this->up);
    //Rotate everything pitch rads about right vector
    glm::vec3 look = rotate(this->look, -pitch, this->right);
    glm::vec3 up = rotate(this->up, -pitch, this->right);
    glm::vec3 right = this->right;
    if (stabilise)
    {
        //Right is perpendicular to look and pureUp
        right = cross(look, this->pureUp);
        //Stabilised up is perpendicular to right and look
        up = cross(right, look);
        //Don't let look get too close to pure up, else we will spin
        if (abs(dot(look, this->pureUp)) > 0.98)
            return;
    }
    //Commit changes
    this->look = normalize(look);
    this->right = normalize(right);
    this->up = normalize(up);
    this->updateViews();
}
void FlyCamera::move(float distance){
    eye += look*distance;
    this->updateViews();
}
void FlyCamera::strafe(float distance){
    eye += right*distance;
    this->updateViews();
}
void FlyCamera::ascend(float distance){
	eye += pureUp*distance;
    this->updateViews();
}
void FlyCamera::roll(float roll){
	pureUp = normalize(rotate(pureUp, roll, look));
	right = normalize(rotate(right, roll, look));
	up = normalize(rotate(up, roll, look));
	this->updateViews();
}
void FlyCamera::setStabilise(bool stabilise){
	this->stabilise = stabilise;
}
void FlyCamera::gluLookAt(){
    GL_CALL(::gluLookAt(
        eye.x, eye.y, eye.z,
        eye.x + look.x, eye.y + look.y, eye.z + look.z,
        up.x, up.y, up.z
        ));
}
void FlyCamera::skyboxGluLookAt() const
{
    GL_CALL(::gluLookAt(
        0, 0, 0,
        look.x, look.y, look.z,
        up.x, up.y, up.z
        ));
}
glm::vec3 FlyCamera::getLook() const{
    return look;
}
glm::vec3 FlyCamera::getUp() const{
    return up;
}
glm::vec3 FlyCamera::getPureUp() const{
    return pureUp;
}
glm::vec3 FlyCamera::getRight() const{
    return right;
}
void FlyCamera::updateViews(){
	viewMat = glm::lookAt(eye, eye + look, up);
	skyboxViewMat = glm::lookAt(glm::vec3(0), look, up);
}