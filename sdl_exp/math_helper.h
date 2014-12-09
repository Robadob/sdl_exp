#pragma once
class math_helper
{
public:
	math_helper();
	~math_helper();

	static double toRadians(double degrees);
	static double clamp(double value, double min, double max);
	static const double PI;
};

