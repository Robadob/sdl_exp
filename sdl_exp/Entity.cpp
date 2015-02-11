#include "Entity.h"

#include <iostream>
#include <fstream>
#include <string>

/**
 * @param modelPath Path to .obj format model file
 * @param modelScale World size to scale the longest direction (in the x, y or z) axis of the model to fit
**/
Entity::Entity(const char *modelPath, float modelScale)
	: vertices(0)
	, normals(0)
	, faces(0)
	, vertices_vbo(0)
	, faces_vbo(0)
	, SCALE(modelScale)
	, material(nullptr)
{
	loadModelFromFile(modelPath, modelScale);
	//Vertex Buffer Objects
	createVertexBufferObject(&vertices_vbo, GL_ARRAY_BUFFER, v_count*sizeof(float3)*2);//vertices+norms
	createVertexBufferObject(&faces_vbo, GL_ELEMENT_ARRAY_BUFFER, f_count*sizeof(int3));
	bindVertexBufferData();
}
Entity::~Entity()
{
	freeModel();
	if (this->material != NULL){
		delete this->material;
	}
	
}
/**
 * Calls the necessary code to render the entities model
**/
void Entity::render()
{
	//Set vertex buffer, and init pointers to vertices, normals
	glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char *)NULL + (v_count*sizeof(float3))));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo);

	// If we have a 
	if (this->material != NULL){
		//this->material->useMaterial();
	}
	else {
		//Red
		glColor4f(1.0, 0.0, 0.0, 1.0);
	}
	glPushMatrix();
		glDrawElements(GL_TRIANGLES, f_count * 3, GL_UNSIGNED_INT, 0);
	glPopMatrix();

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void Entity::renderInstances(int instanceCount)
{
	//Set vertex buffer, and init pointers to vertices, normals
	glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char *)NULL + (v_count*sizeof(float3))));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo);

	// If we have a 
	if (this->material != NULL){
		//this->material->useMaterial();
	}
	else {
		//Red
		glColor4f(1.0, 0.0, 0.0, 1.0);
	}
	glPushMatrix();
		glDrawElementsInstanced(GL_TRIANGLES, f_count * 3, GL_UNSIGNED_INT, 0, instanceCount);
	glPopMatrix();

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

}


/**
 * Creates a vertex buffer object of the specified size
**/
void Entity::createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size)
{
	glGenBuffers( 1, vbo);
	glBindBuffer( target, *vbo);

	glBufferData( target, size, 0, GL_STATIC_DRAW);

	glBindBuffer( target, 0);

	checkGLError();
}
/**
 * Deallocates the provided vertex buffer
**/
void Entity::deleteVertexBufferObject(GLuint *vbo)
{
	glDeleteBuffers(1, vbo);
}
/**
 * Binds model primitive data to the vertex buffer objects
**/
void Entity::bindVertexBufferData()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
	glBufferData(GL_ARRAY_BUFFER, v_count*sizeof(float3)*2, vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, f_count*sizeof(int3), faces, GL_DYNAMIC_DRAW);
}
/**
 * Loads and scales the specified model into this class's primitive storage
 * @note Loading of vertex normals was disabled to suit some models that have non matching counts of vertices and vertex normals
**/
void Entity::loadModelFromFile(const char *path, float modelScale)
{
	const char* VERTEX_IDENTIFIER = "v";
	const char* VERTEX_NORMAL_IDENTIFIER = "vn";
	const char* FACE_IDENTIFIER = "f";
	const char* MTLLIB_IDENTIFIER = "mtllib";
	const char* USEMTL_IDENTIFEIR = "usemtl";
	//Counters
	int vertices_read = 0; 
	int normals_read = 0; 
	int faces_read = 0;
	//Placeholders
	char buffer[100];
	float x,y,z;
	int f1a, f1b, f2a, f2b, f3a, f3b;
	char materialFile[100] = ""; // @todo - this could be improved;
	char materialName[100] = ""; // @todo - this could be improved;
	//Open file
	FILE* file = fopen(path, "r");
	if (file == NULL){
		printf("Could not open model '%s'!\n", path);
		return;
	}
	//Count vertices/faces
	while (!feof(file)) {
		if (fscanf(file, "%s", buffer) == 1){
			if(strcmp(buffer, VERTEX_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &x, &y, &z) == 3)
					vertices_read++;
			}else if(strcmp(buffer, VERTEX_NORMAL_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &x, &y, &z) == 3)
					normals_read++;
			}else if(strcmp(buffer, FACE_IDENTIFIER) == 0){
				if (fscanf(file, "%i//%i %i//%i %i//%i", &f1a, &f1b, &f2a, &f2b, &f3a, &f3b) == 6)
					faces_read++;
			}
		}
	}
	//Create a temp space to cache normals
	float3 *t_normals = (float3*)malloc(normals_read*sizeof(float3));
	//Set/Reset vars
	v_count = vertices_read;
	f_count = faces_read;	
	vertices_read = 0; 
	normals_read = 0; 
	faces_read = 0;
	clearerr(file);
	fseek(file, 0, SEEK_SET);
	//Allocate
	allocateModel();

	//Iterate file, reading in vertices, normals & faces
	while (!feof(file)) {
		if (fscanf(file, "%s", buffer) == 1){
			if(strcmp(buffer, VERTEX_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &x, &y, &z) == 3){
					vertices[vertices_read][0] = x;
					vertices[vertices_read][1] = y;
					vertices[vertices_read][2] = z;
					vertices_read++;
				}else{
					printf("Incomplete vertex data\n");
				}
			}else if(strcmp(buffer, VERTEX_NORMAL_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &x, &y, &z) == 3){
					t_normals[normals_read][0] = x;
					t_normals[normals_read][1] = y;
					t_normals[normals_read][2] = z;
					normals_read++;
				}else{
					printf("Incomplete vertex normal data\n");
				}
			}else if(strcmp(buffer, FACE_IDENTIFIER) == 0){
				//expect 3 vertices
				if (fscanf(file, "%i//%i %i//%i %i//%i", &f1a, &f1b, &f2a, &f2b, &f3a, &f3b) == 6){
					faces[faces_read][0] = f1a-1;
					faces[faces_read][1] = f2a-1;
					faces[faces_read][2] = f3a-1;
					//Copy across all referenced normals
					normals[f1a-1][0] = t_normals[f1b-1][0];
					normals[f1a-1][1] = t_normals[f1b-1][1];
					normals[f1a-1][2] = t_normals[f1b-1][2];

					normals[f2a-1][0] = t_normals[f2b-1][0];
					normals[f2a-1][1] = t_normals[f2b-1][1];
					normals[f2a-1][2] = t_normals[f2b-1][2];

					normals[f3a-1][0] = t_normals[f3b-1][0];
					normals[f3a-1][1] = t_normals[f3b-1][1];
					normals[f3a-1][2] = t_normals[f3b-1][2];
					faces_read++;
				}else{
					printf("Incomplete face data\n");
				}	
			}
			else if (strcmp(buffer, MTLLIB_IDENTIFIER) == 0){
				if (fscanf(file, "%s", &materialFile) != 1){
					printf("Incomplete material filename");
					// reset materialFile
					materialFile[0] = '\0';
				}
			}
			else if (strcmp(buffer, USEMTL_IDENTIFEIR) == 0){
				if (fscanf(file, "%s", &materialName) != 1){
					printf("Incomplete material name");
					// reset materialName
					materialName[0] = '\0';
				}
			}
		}
	}
	if (materialFile[0] != '\0' && materialName != '\0'){
		// Materials are in use, attempt to load the material file
		loadMaterialFromFile(path, materialFile, materialName);
	}


	printf("%s: %i Vertices, %i V-Normals & %i Faces were loaded.\n",path,vertices_read,normals_read,faces_read);
	if(modelScale>0)
	{
		float3 minVert = {100000,100000,100000};
		float3 maxVert = {-100000,-100000,-100000};
		for(int i=0;i<vertices_read;i++)
		{
			minVert[0]=(minVert[0]<vertices[i][0])?minVert[0]:vertices[i][0];
			maxVert[0]=(maxVert[0]>vertices[i][0])?maxVert[0]:vertices[i][0];
		
			minVert[1]=(minVert[1]<vertices[i][1])?minVert[1]:vertices[i][1];
			maxVert[1]=(maxVert[1]>vertices[i][1])?maxVert[1]:vertices[i][1];

			minVert[2]=(minVert[2]<vertices[i][2])?minVert[2]:vertices[i][2];
			maxVert[2]=(maxVert[2]>vertices[i][2])?maxVert[2]:vertices[i][2];
		}
	
		printf("Original Model Dimensions(%0.4f,%0.4f,%0.4f)",maxVert[0],maxVert[1],maxVert[2]);
		//maxVert-=minVert;
		maxVert[0]-=minVert[0];	maxVert[1]-=minVert[1];	maxVert[2]-=minVert[2];
		float longestAxis = (maxVert[0]>maxVert[1])?maxVert[0]:maxVert[1];
		longestAxis = (longestAxis>maxVert[2])?longestAxis:maxVert[2];
		printf(", Scaling model by: %0.4f\n",modelScale/longestAxis);
		scaleModel(modelScale/longestAxis);
	}
	fclose(file);
	free(t_normals);
}

// @todo - this only works with 1 material per file.
void Entity::loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName){
	// Figure out the actual filepath, obj path dir but with matrial filename on the end.
	std::string objectPath = objPath;
	unsigned found = objectPath.find_last_of('/');
	std::string materialPath = objectPath.substr(0, found).append("/").append(materialFilename);

	//Open file
	FILE* file = fopen(materialPath.c_str(), "r");
	if (file == NULL){
		printf("Could not open ,material '%s'!\n", materialPath.c_str());
		return;
	}
	// Prep vars for storing mtl properties
	char buffer[100];
	char temp[100];
	float r, g, b;
	int i;

	// identifier strings
	const char* MATERIAL_NAME_IDENTIFIER = "newmtl";
	const char* AMBIENT_IDENTIFIER = "Ka";
	const char* DIFFUSE_IDENTIFIER = "Kd";
	const char* SPECULAR_IDENTIFIER = "Ks";
	const char* SPECULAR_EXPONENT_IDENTIFIER = "Ns";
	const char* DISSOLVE_IDENTIFEIR = "d"; //alpha
	const char* ILLUMINATION_MODE_IDENTIFIER = "illum";

	this->material = new Material();

	// iter file
	while (!feof(file)) {
		if (fscanf(file, "%s", buffer) == 1){
			if (strcmp(buffer, MATERIAL_NAME_IDENTIFIER) == 0){
				if (fscanf(file, "%s", &temp) == 1){
					//this->material->materialName = temp; // @todo
				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, AMBIENT_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
					this->material->ambient.xyzw(r, g, b, 1.0);
				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, DIFFUSE_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
					this->material->diffuse.xyzw(r, g, b, 1.0);

				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, SPECULAR_IDENTIFIER) == 0){
				if (fscanf(file, "%f %f %f", &r, &g, &b) == 3){
					this->material->specular.xyzw(r, g, b, 1.0);

				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, SPECULAR_EXPONENT_IDENTIFIER) == 0){
				if (fscanf(file, "%f", &r) == 1){
					this->material->shininess = r;
				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, DISSOLVE_IDENTIFEIR) == 0) {
				if (fscanf(file, "%f", &r) == 1){
					this->material->dissolve = r;
				}
				else {
					printf("Bad material file...");
				}
			}
			else if (strcmp(buffer, ILLUMINATION_MODE_IDENTIFIER) == 0) {
				if (fscanf(file, "%d", &i) == 1){
					//@todo  ignore this for now.

					//this->material->illuminationMode = i;
				}
				else {
					printf("Bad material file...");
				}
			}

		}
	}
	
}

/**
 * Allocates the storage for model primitives
**/
void Entity::allocateModel()
{
	vertices = (float3*)malloc(v_count*sizeof(float3)*2);
	normals = vertices+v_count;//Arrays are continuous
	faces = (int3*)malloc(f_count*sizeof(int3));
}
/**
 * Deallocates the storage for model primitives
**/
void Entity::freeModel()
{
	if(vertices)
		free(vertices);
	if(faces)
		free(faces);
}
/**
 * Scales the vertices to fit the provided scale
**/
void Entity::scaleModel(float modelScale)
{
	for(int i =0; i<v_count;i++)
	{
		vertices[i][0]*=modelScale;
		vertices[i][1]*=modelScale;
		vertices[i][2]*=modelScale;
	}
}
//Not linking with the necessary libs to convert glu error codes to strings :(
void Entity::checkGLError(){
	int err;
	if((err = glGetError()) != GL_NO_ERROR)
	{
		//const char* message = (const char*)gluErrorString(err);
		//fprintf(stderr, "OpenGL Error Occured : %s\n", message
		printf("OpenGL Error Occured: %i\n",err);// : %s\n", message);
	}
}