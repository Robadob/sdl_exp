#include "Vec3F.h"
#include <math.h>


Vec3F::Vec3F(float x, float y, float z){
	this->x = x;
	this->y = y;
	this->z = z;
}


Vec3F::~Vec3F(){
}

float Vec3F::length(){
	return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
}

void Vec3F::normalize(){
	float length = this->length();
	if (length != 0){
		this->x /= length;
		this->y /= length;
		this->z /= length;
	}
}

void Vec3F::xyz(float x, float y, float z){
	this->x = x;
	this->y = y;
	this->z = z;
}


// Operators
Vec3F operator+(const Vec3F &a, const Vec3F &b){
	return Vec3F(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vec3F operator-(const Vec3F &a, const Vec3F &b){
	return Vec3F(a.x - b.x, a.y - b.y, a.z - b.z);
}
// Cross product, maybe change to named function.
Vec3F operator*(const Vec3F &a, const Vec3F &b){
	return Vec3F(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
		);
}


Vec3F operator*(const Vec3F &a, const float &b){
	return Vec3F(
		a.x * b,
		a.y * b,
		a.z * b
		);
}