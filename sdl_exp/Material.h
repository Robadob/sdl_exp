#pragma once

#include "Vec3F.h"

class Material
{
public:
	Material();
	Material(Vec3F ambient, Vec3F diffuse, Vec3F specular, Vec3F emission, float shininess, float dissolve);
	~Material();

	Vec3F ambient;
	Vec3F diffuse;
	Vec3F specular;
	Vec3F emission; // Unused;
	float shininess;
	float dissolve;

	void printToConsole();
};

