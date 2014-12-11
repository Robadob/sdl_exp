#pragma once
class Vec3D
{
public:
	Vec3D(double x = 0.0, double y = 0.0, double z = 0.0);
	~Vec3D();
	// Public member vars
	double x;
	double y;
	double z;

	double length();
	void normalize();
	void xyz(double x, double y, double z);
	
	// Operators
	friend Vec3D operator+(const Vec3D &a, const Vec3D &b);
	friend Vec3D operator-(const Vec3D &a, const Vec3D &b);
	friend Vec3D operator*(const Vec3D &a, const Vec3D &b);

	friend Vec3D operator*(const Vec3D &a, const double &b);

};

