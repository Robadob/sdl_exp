#pragma once
class Axis
{
public:
	Axis(double length = 1.0);
	~Axis();

	void render();

private:
	double length;
};

