#include "Vec4F.h"
#include <math.h>

//@todo make this an extstenion of 3f?

Vec4F::Vec4F(float x, float y, float z, float w){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}


Vec4F::~Vec4F(){
	
}

float Vec4F::length(){
	return sqrt(pow(this->w, 2) + pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
}

void Vec4F::normalize(){
	float length = this->length();
	if (length != 0){
		this->w /= length;
		this->x /= length;
		this->y /= length;
		this->z /= length;
	}
}

void Vec4F::xyzw(float x, float y, float z, float w){
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;

}

// Operators
Vec4F operator+(const Vec4F &a, const Vec4F &b){
	return Vec4F(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
Vec4F operator-(const Vec4F &a, const Vec4F &b){
	return Vec4F(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
// Cross product, maybe change to named function.
Vec4F operator*(const Vec4F &a, const Vec4F &b){
	return Vec4F(
		a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
		a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
		a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
		a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
	);
}


Vec4F operator*(const Vec4F &a, const float &b){
	return Vec4F(
		a.x * b,
		a.y * b,
		a.z * b,
		a.w * b
	);
}