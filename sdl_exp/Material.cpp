#include "Material.h"
#include <stdio.h>
#include "GL\glew.h"


#define DEFAULT_SHININESS 32

Material::Material() : ambient(Vec4F(0, 0, 0, 1)), diffuse(Vec4F(0, 0, 0, 1)), specular(Vec4F(0, 0, 0, 1)), emission(Vec4F(0, 0, 0, 1)), shininess(DEFAULT_SHININESS), dissolve(1){

}

Material::Material(Vec4F ambient, Vec4F diffuse, Vec4F specular, Vec4F emission, float shininess, float dissolve)
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

void Material::useMaterial(){
	// Set material properties from stored vars.
	float amb[4] = { this->ambient.x, this->ambient.y, this->ambient.z, this->ambient.w};
	float diff[4] = { this->diffuse.x, this->diffuse.y, this->diffuse.z, this->diffuse.w };
	float spec[4] = { this->specular.x, this->specular.y, this->specular.z, this->specular.w };
	
	// Call gl material fns
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &this->shininess);
	
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