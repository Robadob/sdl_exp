#include "Quaternion.h"

#include <math.h>

// Constructor(s)
Quaternion::Quaternion(){
}

Quaternion::Quaternion(double x, double y, double z, double w){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

// Deconstructor
Quaternion::~Quaternion(){
}

// Public methods

double Quaternion::length(){
	return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2) + pow(this->w, 2));
}

void Quaternion::normalize(){
	double length = this->length();
	this->x /= length;
	this->y /= length;
	this->z /= length;
	this->w /= length;
}

Quaternion Quaternion::conjugate(){
	return Quaternion(-this->x, -this->y, -this->z, this->w);
}

// Overloaded operators?
Quaternion operator*(const Quaternion &a, const Quaternion &b){
	double x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	double y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;;
	double z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;;
	double w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;;
	return Quaternion(x, y, z, w);
}

// Private methods

