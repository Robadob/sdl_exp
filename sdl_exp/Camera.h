#pragma once

#include "gl\glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "Vec3D.h"
#include <glm\glm.hpp>
#include <glm/gtx/transform.hpp>


class Camera
{
public:
	Camera(double theta = 30.0, double phi = 10.0, double x = 0, double y = 0, double z = 0);
	~Camera();

	void updateThetaPhi(double thetaInc, double phiInc);
	void move(double distance);
	void strafe(double distance);
	void ascend(double distance);
	void updateWorldPosition(glm::dvec3 inc);
	void calcTargetUp();
	glm::mat4 view(); // Calls glu look at

private:
	glm::dvec3 target;
	glm::dvec3 eye;
	glm::dvec3 up;
	glm::dvec3 worldPosition;
	
	double theta;
	double phi;
};

