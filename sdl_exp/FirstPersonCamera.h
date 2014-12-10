#pragma once

#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#define xIndex 0
#define yIndex 1
#define zIndex 2

class FirstPersonCamera
{
public:
	FirstPersonCamera();
	~FirstPersonCamera();

	void view();
	void move(double distance);
	void strafe(double distance);
	void updatePitchYawRoll(double pitch, double yaw, double roll);
	void calcDirection();

	double position[3];
	double roll = 0.0;
	double pitch = 0.0;
	double yaw = 0.0;
	double direction[3];
};

