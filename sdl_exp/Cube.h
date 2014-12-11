#pragma once
class Cube
{
public:
	Cube(double scale = 1.0);
	~Cube();

	void render();

private:
	double scale;
};

