#include "FirstPersonCamera.h"
#include <stdio.h>
#include <cmath>

#define DEGREE_LIMIT 360


FirstPersonCamera::FirstPersonCamera(): pitch(180){
	this->position[xIndex] = 0;
	this->position[yIndex] = -1;
	this->position[zIndex] = -10;

	this->direction[xIndex] = 0;
	this->direction[yIndex] = 0;
	this->direction[zIndex] = 0;
}


FirstPersonCamera::~FirstPersonCamera(){
}

void FirstPersonCamera::view(){
	glLoadIdentity();
	glRotated(this->pitch, 1.0, 0.0, 0.0);
	glRotated(this->yaw, 0.0, 1.0, 0.0);
	glRotated(this->roll, 0.0, 0.0, 1.0);
	glTranslated(-this->position[xIndex], -this->position[yIndex], -this->position[zIndex]);

	//printf("glTranslated(%f, %f, %f);\n",  - this->position[xIndex], -this->position[yIndex], -this->position[zIndex]);
	//printf("direction(%f, %f, %f);\n", this->direction[xIndex], this->direction[yIndex], this->direction[zIndex]);
}

void FirstPersonCamera::move(double distance){
	this->position[xIndex] += this->direction[xIndex] * distance;
	this->position[yIndex] += this->direction[yIndex] * distance;
	this->position[zIndex] += this->direction[zIndex] * distance;
}

void FirstPersonCamera::strafe(double distance){

}



void FirstPersonCamera::updatePitchYawRoll(double pitch, double yaw, double roll){
	this->pitch += pitch;
	this->yaw += yaw;
	this->roll += roll;

	// Ensure values are between 0 and 360?
	if (this->pitch > DEGREE_LIMIT){
		this->pitch = remainder(this->pitch, DEGREE_LIMIT);
	}
	if (this->yaw > DEGREE_LIMIT){
		this->yaw = remainder(this->yaw, DEGREE_LIMIT);
	}
	if (this->roll > DEGREE_LIMIT){
		this->roll = remainder(this->roll, DEGREE_LIMIT);
	}

	this->calcDirection();
}

// @todo - the order of axis is wrong, switch y and z? or fall back to moving the spherical system...
void FirstPersonCamera::calcDirection(){
	// Using pitch and yaw get the direction vector
	this->direction[xIndex] = cos(this->yaw) * cos(this->pitch);
	this->direction[yIndex] = sin(this->yaw) * cos(this->pitch);
	this->direction[zIndex] = sin(this->pitch);
	printf("direction(%f, %f, %f);\n", this->direction[xIndex], this->direction[yIndex], this->direction[zIndex]);

}