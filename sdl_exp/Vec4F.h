#pragma once
class Vec4F
{
public:
	Vec4F(float x = 0.0, float y = 0.0, float z = 0.0, float w = 0.0);
	~Vec4F();
	// Public member vars
	float x;
	float y;
	float z;
	float w;

	float length();
	void normalize();
	void xyzw(float x, float y, float z, float w);

	// Operators
	friend Vec4F operator+(const Vec4F &a, const Vec4F &b);
	friend Vec4F operator-(const Vec4F &a, const Vec4F &b);
	friend Vec4F operator*(const Vec4F &a, const Vec4F &b);

	friend Vec4F operator*(const Vec4F &a, const float &b);

};

