#include "math_helper.h"

const double math_helper::PI = 3.14159265358979323846; // Why is there no pi const in cpp libs? =s


math_helper::math_helper()
{
}


math_helper::~math_helper()
{
}

double math_helper::toRadians(double degrees){
	return degrees * (math_helper::PI / 180);
}

double math_helper::clamp(double value, double min, double max){
	if (value >= min && value <= max){
		return value;
	} else if (value < min){
		return min;
	} else {
		return max;
	}
}
