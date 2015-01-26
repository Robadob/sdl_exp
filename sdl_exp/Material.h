#pragma once

#include "Vec4F.h"

class Material
{
public:
	Material();
	Material(Vec4F ambient, Vec4F diffuse, Vec4F specular, Vec4F emission, float shininess, float dissolve);
	~Material();

	Vec4F ambient;
	Vec4F diffuse;
	Vec4F specular;
	Vec4F emission; // Unused;
	float shininess;
	float dissolve;

	void useMaterial();
	void printToConsole();
};

