#include "Camera.h"
#include <cmath>
#include "math_helper.h"
#include <stdio.h>


Camera::Camera(double theta, double phi, double radius){
	this->theta = theta;
	this->phi = phi;
	this->radius = radius;

	this->eye[xIndex] = 10.0;
	this->eye[yIndex] = 10.0;
	this->eye[zIndex] = 10.0;

	this->lookAt[xIndex] = 0.0;
	this->lookAt[yIndex] = 0.0;
	this->lookAt[zIndex] = 0.0;

	this->up[xIndex] = 0.0;
	this->up[yIndex] = 1.0;
	this->up[zIndex] = 0.0;

	this->calcEyePosition();
}

Camera::~Camera(){

}

void Camera::updateThetaPhi(double thetaInc, double phiInc){
	this->theta += thetaInc;
	this->phi += phiInc;
	this->calcEyePosition();
}

void Camera::updateRadius(double radiusInc){
	this->radius += radiusInc;
	this->calcEyePosition();
}

void Camera::calcEyePosition(){

	// Limit theta to between -90 and 90 degrees

	double phiLimit = math_helper::PI / 2;
	this->phi = math_helper::clamp(this->phi, -phiLimit, phiLimit);

	double cy = cos(this->theta);
	double sy = sin(this->theta);
	double cz = cos(this->phi);
	double sz = sin(this->phi);

	


	this->eye[xIndex] = this->radius * cy * cz;
	this->eye[yIndex] = this->radius * sz;
	this->eye[zIndex] = -this->radius * sy * cz;

	this->up[xIndex] = -cy * sz;
	this->up[yIndex] = cz;
	this->up[zIndex] = sy * sz;

	// Invert the whole vector when the camera is upside down
	if (this->up[yIndex] < 0){
		this->up[xIndex] = -this->up[xIndex];
		this->up[yIndex] = -this->up[yIndex];
		this->up[zIndex] = -this->up[zIndex];
	}

	/*printf("\n");
	printf("t: %f, p: %f, r: %f \n", this->theta, this->phi, this->radius);
	printf("cy: %f, sy: %f, cz: %f, sz: %f \n", cy, sy, cz, sz);
	printf("eye: [%f, %f, %f]\n", this->eye[xIndex], this->eye[yIndex], this->eye[zIndex]);
	printf("lookAt: [%f, %f, %f]\n", this->lookAt[xIndex], this->lookAt[yIndex], this->lookAt[zIndex]);
	printf("up: [%f, %f, %f]\n", this->up[xIndex], this->up[yIndex], this->up[zIndex]);
	printf("\n");*/
}

void Camera::view(){
	gluLookAt(
		this->eye[xIndex], this->eye[yIndex], this->eye[zIndex],
		this->lookAt[xIndex], this->lookAt[yIndex], this->lookAt[zIndex],
		this->up[xIndex], this->up[yIndex], this->up[zIndex]
		);

	/*printf(
		"%f, %f, %f\n %f, %f, %f\n %f, %f, %f\n\n",
		this->eye[xIndex], this->eye[yIndex], this->eye[zIndex],
		this->lookAt[xIndex], this->lookAt[yIndex], this->lookAt[zIndex],
		this->up[xIndex], this->up[yIndex], this->up[zIndex]
	);*/
	//gluLookAt(10, 10, 10, 0, 0, 0, 0, 1, 0);
}
