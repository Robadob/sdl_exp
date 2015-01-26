#pragma once
class Vec3F
{
public:
	Vec3F(float x = 0.0, float y = 0.0, float z = 0.0);
	~Vec3F();
	// Public member vars
	float x;
	float y;
	float z;

	float length();
	void normalize();
	void xyz(float x, float y, float z);

	// Operators
	friend Vec3F operator+(const Vec3F &a, const Vec3F &b);
	friend Vec3F operator-(const Vec3F &a, const Vec3F &b);
	friend Vec3F operator*(const Vec3F &a, const Vec3F &b);

	friend Vec3F operator*(const Vec3F &a, const float &b);

};

