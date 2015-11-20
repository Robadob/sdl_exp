#include "Camera.h"
#include <stdio.h>

Camera::Camera() : 
    Camera(glm::vec3(0, 0, 0))
{
}
Camera::Camera(glm::vec3 eye) : 
    eye(eye), 
    look(normalize(glm::vec3(eye.x + 1.0, eye.y, eye.z))), 
    up(normalize(glm::vec3(eye.x, eye.y + 1.0, eye.z))), 
    right(normalize(glm::vec3(eye.x, eye.y, eye.z + 1.0)))
{
   
}
Camera::Camera(glm::vec3 eye, float yaw, float pitch) : 
    Camera(eye)
{
    turn(yaw, pitch);
}

Camera::~Camera(){
}
void Camera::turn(float yaw, float pitch){
    //Rotate everything yaw rads about up vector
    look = normalize(rotate(this->look, -yaw, this->up));
    right = normalize(rotate(this->right, -yaw, this->up));
    //Rotate everything pitch rads about right vector
    look = normalize(rotate(this->look, -pitch, this->right));
    up = normalize(rotate(this->up, -pitch, this->right));
}

void Camera::move(float distance){
    //Move eye specified distance along the look vector
    eye += look*distance;
}
void Camera::roll(float roll)
{
    //Rotate everything roll rads about look vector
    right = normalize(rotate(right, roll, look));
    up = normalize(rotate(up, roll, look));
}
void Camera::strafe(float distance){
    //Move eye specified distance along the right vector
    eye += right*distance;
}

void Camera::ascend(float distance){
    //Move eye specified distance along the up vector
    eye += up*distance;
}
glm::mat4 Camera::view(){
#ifdef GLU_LOOK_AT
  gluLookAt(
      eye.x,eye.y,eye.z,
      eye.x + look.x, eye.y + look.y, eye.z + look.z,
      up.x, up.y, up.z
      );
#endif
	glm::mat4 view = glm::lookAt(eye, eye+look, up);
	return view;
}