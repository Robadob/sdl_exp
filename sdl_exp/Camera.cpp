#include "Camera.h"
#include <cmath>
#include "math_helper.h"
#include <stdio.h>

# define SPHERE_RADIUS 1.0

Camera::Camera(double theta, double phi, double x, double y, double z): target(glm::dvec3(0, 0, 0)), eye(glm::dvec3(0, 0, 0)), up(glm::dvec3(0.0, 1.0, 0.0)){
	this->theta = theta;
	this->phi = phi;
	this->worldPosition = glm::dvec3(x, y, z);

	this->calcTargetUp();
}

Camera::~Camera(){
}

void Camera::updateThetaPhi(double thetaInc, double phiInc){
	this->theta += thetaInc;
	this->phi += phiInc;
	this->calcTargetUp();
}

void Camera::move(double distance){
	// Move specified distance along the look at vector in world co ordinate space
	glm::dvec3 ahead = glm::normalize(this->eye - this->target);
	this->updateWorldPosition(ahead * distance);
}

void Camera::strafe(double distance){
	// Find the right vector, the cross product of the vectors between pos-eye and up-pos
	glm::dvec3 targetEye = this->eye - this->target;
	glm::dvec3 upTarget = this->target - this->up;
	glm::dvec3 right = glm::normalize(glm::cross(targetEye, upTarget));
	this->updateWorldPosition(right * distance);
}

void Camera::ascend(double distance){
	// Increment the y world co ord, maybe switch to traveling along the up vector.
	this->updateWorldPosition(glm::dvec3(0, 1, 0) * distance);
}

void Camera::updateWorldPosition(glm::dvec3 inc){
	this->worldPosition = this->worldPosition + inc;
}


void Camera::calcTargetUp(){
	// Limit theta to between -90 and 90 degrees
	const double phiLimit = math_helper::PI / 2;
	this->phi = math_helper::clamp(this->phi, -phiLimit, phiLimit);

	double cy = cos(this->theta);
	double sy = sin(this->theta);
	double cz = cos(this->phi);
	double sz = sin(this->phi);

	
	this->target = glm::dvec3(
		SPHERE_RADIUS * cy * cz,
		SPHERE_RADIUS * sz,
		-SPHERE_RADIUS * sy * cz
	);

	this->up = glm::dvec3(
		-cy * sz,
		cz,
		sy * sz
	);

	// Invert the whole vector when the camera is upside down
	if (this->up.y < 0){
		this->up = this->up * -1.0;
	}
}

glm::mat4 Camera::view(){
	//Vec3D worldEye = this->eye + this->worldPosition;
	//Vec3D worldTarget = this->target + this->worldPosition;
	/*gluLookAt(
		worldEye.x, worldEye.y, worldEye.z,
		worldTarget.x, worldTarget.y, worldTarget.z,
		this->up.x, this->up.y, this->up.z
		);*/
	glm::dvec3 worldEye = this->eye + this->worldPosition;
	glm::dvec3 worldTarget = this->target + this->worldPosition;
	glm::mat4 view = glm::lookAt(worldEye, worldTarget, this->up);
	return view;
}
