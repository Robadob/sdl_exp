#include "Vec3D.h"
#include <math.h>


Vec3D::Vec3D(double x, double y, double z){
	this->x = x;
	this->y = y;
	this->z = z;
}


Vec3D::~Vec3D(){
}

double Vec3D::length(){
	return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
}

void Vec3D::normalize(){
	double length = this->length();
	if (length != 0){
		this->x /= length;
		this->y /= length;
		this->z /= length;
	}
}

void Vec3D::xyz(double x, double y, double z){
	this->x = x;
	this->y = y;
	this->z = z;
}

// Operators
Vec3D operator+(const Vec3D &a, const Vec3D &b){
	return Vec3D(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vec3D operator-(const Vec3D &a, const Vec3D &b){
	return Vec3D(a.x - b.x, a.y - b.y, a.z - b.z);
}
// Cross product, maybe change to named function.
Vec3D operator*(const Vec3D &a, const Vec3D &b){
	return Vec3D(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}


Vec3D operator*(const Vec3D &a, const double &b){
	return Vec3D(
		a.x * b,
		a.y * b,
		a.z * b
		);
}