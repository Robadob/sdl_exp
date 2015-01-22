#ifndef __Entity_h__
#define __Entity_h__

#include "GL\glew.h"

typedef float float3[3];
typedef float float4[4];
typedef int int3[3];

struct mtl {
	char* materialName;
	float3 ambient, diffuse, specular;
	float specularExponent, dissolve;
	int illuminationMode;
};

class Entity
{
public:
	/**
	 * @param modelPath Path to .obj format model file
	 * @param modelScale World size to scale the longest direction (in the x, y or z) axis of the model to fit
	**/
	Entity(const char *modelPath, float modelScale=1.0);
	~Entity();
	/**
	 * Calls the necessary code to render the entities model
	**/
	void render();
protected:
	//World scale of the longest side (in the axis x, y or z)
	const float SCALE;
	//Model vertex and face counts
	int v_count, f_count;
	//Primitive data
	float3 *vertices, *normals;
	int3 *faces;
	//Vertex Buffer Objects for rendering
	GLuint vertices_vbo;
	GLuint faces_vbo;
	// Material properties
	mtl material;
	

	/**
	 * Creates a vertex buffer object of the specified size
	**/
	void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size);
	/**
	 * Deallocates the provided vertex buffer
	**/
	void deleteVertexBufferObject(GLuint *vbo);
	/**
	 * Binds model primitive data to the vertex buffer objects
	**/
	void bindVertexBufferData();
	/**
	 * Loads and scales the specified model into this class's primitive storage
	 * @note Loading of vertex normals was disabled to suit some models that have non matching counts of vertices and vertex normals
	**/
	void loadModelFromFile(const char *path, float modelScale);
	void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
	/**
	 * Allocates the storage for model primitives
	**/
	void allocateModel();
	/**
	 * Deallocates the storage for model primitives
	**/
	void freeModel();
	/**
	 * Scales the vertices to fit the provided scale
	**/
	void scaleModel(float modelScale);

	void checkGLError();
};

#endif