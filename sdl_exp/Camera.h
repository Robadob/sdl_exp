#pragma once

#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "Vec3D.h"

class Camera
{
public:
	Camera(double theta = 30.0, double phi = 10.0, double x = 0, double y = 0, double z = 0);
	~Camera();

	void updateThetaPhi(double thetaInc, double phiInc);
	void move(double distance);
	void strafe(double distance);
	void ascend(double distance);
	void updateWorldPosition(Vec3D inc);
	void calcTargetUp();
	void view(); // Calls glu look at

private:
	Vec3D target;
	Vec3D eye;
	Vec3D up;
	Vec3D worldPosition;
	
	double theta;
	double phi;
};

