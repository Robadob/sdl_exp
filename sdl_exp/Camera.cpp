#include "Camera.h"
#include <cmath>
#include "math_helper.h"
#include <stdio.h>

# define SPHERE_RADIUS 1.0

Camera::Camera(double theta, double phi, double x, double y, double z): target(Vec3D()), eye(Vec3D()), up(Vec3D(0.0, 1.0, 0.0)){
	this->theta = theta;
	this->phi = phi;
	this->worldPosition = Vec3D(x, y, z);

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
	Vec3D ahead = this->eye - this->target;
	ahead.normalize();
	this->updateWorldPosition(ahead * distance);
}

void Camera::strafe(double distance){
	// Find the right vector, the cross product of the vectors between pos-eye and up-pos
	Vec3D targetEye = this->eye - this->target;
	Vec3D upTarget = this->target - this->up;
	Vec3D right = targetEye * upTarget;
	right.normalize();
	this->updateWorldPosition(right * distance);
}

void Camera::ascend(double distance){
	// Increment the y world co ord, maybe switch to traveling along the up vector.
	this->updateWorldPosition(Vec3D(0, 1, 0) * distance);
}

void Camera::updateWorldPosition(Vec3D inc){
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

	this->target.xyz(
		SPHERE_RADIUS * cy * cz,
		SPHERE_RADIUS * sz,
		-SPHERE_RADIUS * sy * cz
	);

	this->up.xyz(
		-cy * sz,
		cz,
		sy * sz
	);

	// Invert the whole vector when the camera is upside down
	if (this->up.y < 0){
		this->up = this->up * -1;
	}
}

void Camera::view(){
	Vec3D worldEye = this->eye + this->worldPosition;
	Vec3D worldTarget = this->target + this->worldPosition;

	gluLookAt(
		worldEye.x, worldEye.y, worldEye.z,
		worldTarget.x, worldTarget.y, worldTarget.z,
		this->up.x, this->up.y, this->up.z
		);
}
