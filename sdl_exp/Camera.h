#pragma once

#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#define xIndex 0
#define yIndex 1
#define zIndex 2

class Camera
{
public:
	Camera(double theta = 30.0, double phi = 10.0, double radius = 1.0, double x = 0, double y = 0, double z = 0);
	~Camera();

	void updateThetaPhi(double thetaInc, double phiInc);
	void updateRadius(double radiusInc);
	void move(double distance);
	void strafe(double distance);
	void updateWorldPosition(double xInc, double yInc, double zInc);
	void calcEyePosition();
	void view(); // Calls glu look at

private:
	double eye[3];
	double lookAt[3];
	double up[3];

	double worldPosition[3];
	
	double theta;
	double phi;
	double radius;
};

