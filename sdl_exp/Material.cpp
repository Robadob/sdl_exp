#include "Material.h"
#include <stdio.h>

#define DEFAULT_SHININESS 32

Material::Material() : ambient(Vec3F(0, 0, 0)), diffuse(Vec3F(0, 0, 0)), specular(Vec3F(0, 0, 0)), emission(Vec3F(0, 0, 0)), shininess(DEFAULT_SHININESS), dissolve(1){

}

Material::Material(Vec3F ambient, Vec3F diffuse, Vec3F specular, Vec3F emission, float shininess, float dissolve)
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
	this->emission = emission;
	this->shininess = shininess;
	this->dissolve = dissolve;
}


Material::~Material()
{
}

void Material::printToConsole(){
	printf("Material::\n");
	printf("  > ambient {%f, %f, %f}\n", this->ambient.x, this->ambient.y, this->ambient.z);
	printf("  > diffuse {%f, %f, %f}\n", this->diffuse.x, this->diffuse.y, this->diffuse.z);
	printf("  > specular {%f, %f, %f}\n", this->specular.x, this->specular.y, this->specular.z);
	printf("  > emission {%f, %f, %f}\n", this->emission.x, this->emission.y, this->emission.z);
	printf("  > shininess %f\n", this->shininess);
	printf("/Material\n\n");

}