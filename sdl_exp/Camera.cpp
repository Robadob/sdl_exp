#include "Camera.h"
#include <cmath>
#include "math_helper.h"
#include <stdio.h>


Camera::Camera(double theta, double phi, double radius, double x, double y, double z){
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

	this->worldPosition[xIndex] = x;
	this->worldPosition[yIndex] = y;
	this->worldPosition[zIndex] = z;

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

//@todo tidy this up with some form of vector class
void Camera::move(double distance){
	// Move specified distance along the look at vector in world co ordinate space

	// Extract the vector components,
	double vecX = this->lookAt[xIndex] - this->eye[xIndex];
	double vecY = this->lookAt[yIndex] - this->eye[yIndex];
	double vecZ = this->lookAt[zIndex] - this->eye[zIndex];

	// Normalize
	double vecLen = sqrt(pow(vecX, 2) + pow(vecY, 2) + pow(vecZ, 2));
	vecX /= vecLen;
	vecY /= vecLen;
	vecZ /= vecLen;


	// Move distance units along said vector
	this->updateWorldPosition(distance * vecX, distance * vecY, distance * vecZ);

}

void Camera::strafe(double distance){
	// Find the right vector, the cross product of the vectors between pos-lookAt and up-pos

	double plX = this->lookAt[xIndex] - this->eye[xIndex];
	double plY = this->lookAt[yIndex] - this->eye[yIndex];
	double plZ = this->lookAt[zIndex] - this->eye[zIndex];

	double uppX = this->eye[xIndex] - this->up[xIndex];
	double uppY = this->eye[yIndex] - this->up[yIndex];
	double uppZ = this->eye[zIndex] - this->up[zIndex];

	// Temp inline cross product
	double crossX = plY*uppZ - plZ*uppY;
	double crossY = plZ*uppX - plX*uppZ;
	double crossZ = plX*uppY - plY*uppX;

	// Should probs normalize here? 
	double crossLen = sqrt(pow(crossX, 2) + pow(crossY, 2) + pow(crossZ, 2));
	crossX /= crossLen;
	crossY /= crossLen;
	crossZ /= crossLen;

	// Move distance units along cross vector
	this->updateWorldPosition(distance * crossX, distance * crossY, distance * crossZ);


}

void Camera::updateWorldPosition(double xInc, double yInc, double zInc){
	this->worldPosition[xIndex] += xInc;
	this->worldPosition[yIndex] += yInc;
	this->worldPosition[zIndex] += zInc;
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
		this->eye[xIndex] + this->worldPosition[xIndex], this->eye[yIndex] + this->worldPosition[yIndex], this->eye[zIndex] + this->worldPosition[zIndex],
		this->lookAt[xIndex] + this->worldPosition[xIndex], this->lookAt[yIndex] + this->worldPosition[yIndex], this->lookAt[zIndex] + this->worldPosition[zIndex],
		this->up[xIndex], this->up[yIndex], this->up[zIndex]
		);

	/*printf("gluLookAt(\n  %f, %f, %f,\n  %f, %f, %f,\n  %f, %f, %f,\n);\n",
		this->eye[xIndex] + this->worldPosition[xIndex], this->eye[yIndex] + this->worldPosition[yIndex], this->eye[zIndex] + this->worldPosition[zIndex],
		this->lookAt[xIndex] + this->worldPosition[xIndex], this->lookAt[yIndex] + this->worldPosition[yIndex], this->lookAt[zIndex] + this->worldPosition[zIndex],
		this->up[xIndex], this->up[yIndex], this->up[zIndex]
	);*/
}
