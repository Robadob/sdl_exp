#pragma once
class Quaternion
{
public:
	// Constructor(s)
	Quaternion();
	Quaternion(double x, double y, double z, double w);
	// Destructor
	~Quaternion();

	// Public member variables for simple acces, Could have used a struct tbh
	double x;
	double y;
	double z;
	double w;

	double length();
	void normalize();
	Quaternion conjugate();
	
	// Overloaded operators?
	friend Quaternion operator*(const Quaternion &a, const Quaternion &b);


	// Public methods
private:
	// Private methods
};

