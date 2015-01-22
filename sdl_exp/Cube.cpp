#include "Cube.h"
#include <stdio.h>

#define VERTICES_PER_TRIANGLE 3
#define NUM_TRIANGLES 12
#define NUM_VERTICES 24
#define FLOATS_PER_

// @todo - sort this out, could use Vec3D but probs bigger memroy than needed?
struct float3 {
	float x,y,z;
};
struct color {
	float r, g, b, a;
};

Cube::Cube(double scale){
	this->scale = scale;
	// Create vbos
	this->createVBO(&this->vertexVBO, NUM_VERTICES * sizeof(float3));
	this->createVBO(&this->normalVBO, NUM_VERTICES * sizeof(float3));
	this->createVBO(&this->colorVBO, NUM_VERTICES * sizeof(color));
	this->createEAB(&this->elementArrayBuffer, NUM_TRIANGLES * VERTICES_PER_TRIANGLE * sizeof(int));
	// Populate vbos
	this->setVBOData();

}


Cube::~Cube(){
	deleteVBO(&this->vertexVBO);
	deleteVBO(&this->normalVBO);
	deleteVBO(&this->colorVBO);
	deleteEAB(&this->elementArrayBuffer);

}

void Cube::render(){
	glPushMatrix();
		glScaled(this->scale, this->scale, this->scale);
		glBegin(GL_QUADS);


			// Begin drawing the color cube with 6 quads
			// Top face (y = 0.5f)
			// Define vertices in counter-clockwise (CCW) order with normal pointing out
			glColor3f(0.0f, 0.5f, 0.0f);     // Green
			glVertex3f(0.5f, 0.5f, -0.5f);
			glVertex3f(-0.5f, 0.5f, -0.5f);
			glVertex3f(-0.5f, 0.5f, 0.5f);
			glVertex3f(0.5f, 0.5f, 0.5f);

			// Bottom face (y = -0.5f)
			glColor3f(0.5f, 0.5f, 0.0f);     // Orange
			glVertex3f(0.5f, -0.5f, 0.5f);
			glVertex3f(-0.5f, -0.5f, 0.5f);
			glVertex3f(-0.5f, -0.5f, -0.5f);
			glVertex3f(0.5f, -0.5f, -0.5f);

			// Front face  (z = 0.5f)
			glColor3f(0.5f, 0.0f, 0.0f);     // Red
			glVertex3f(0.5f, 0.5f, 0.5f);
			glVertex3f(-0.5f, 0.5f, 0.5f);
			glVertex3f(-0.5f, -0.5f, 0.5f);
			glVertex3f(0.5f, -0.5f, 0.5f);

			// Back face (z = -0.5f)
			glColor3f(0.5f, 0.5f, 0.0f);     // Yellow
			glVertex3f(0.5f, -0.5f, -0.5f);
			glVertex3f(-0.5f, -0.5f, -0.5f);
			glVertex3f(-0.5f, 0.5f, -0.5f);
			glVertex3f(0.5f, 0.5f, -0.5f);

			// Left face (x = -0.5f)
			glColor3f(0.0f, 0.0f, 0.5f);     // Blue
			glVertex3f(-0.5f, 0.5f, 0.5f);
			glVertex3f(-0.5f, 0.5f, -0.5f);
			glVertex3f(-0.5f, -0.5f, -0.5f);
			glVertex3f(-0.5f, -0.5f, 0.5f);

			// Right face (x = 0.5f)
			glColor3f(0.5f, 0.0f, 0.5f);     // Magenta
			glVertex3f(0.5f, 0.5f, -0.5f);
			glVertex3f(0.5f, 0.5f, 0.5f);
			glVertex3f(0.5f, -0.5f, 0.5f);
			glVertex3f(0.5f, -0.5f, -0.5f);
		glEnd();  // End of drawing color-cube
	glPopMatrix();
}

// stolen from flame
void Cube::createVBO(GLuint* vbo, GLuint size)
{
	// create buffer object
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	// initialize buffer object
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//@todo checkGLError();
}

// stolen from flame.
void Cube::deleteVBO(GLuint* vbo)
{
	glBindBuffer(1, *vbo);
	glDeleteBuffers(1, vbo);

	*vbo = 0;
}

//@todo, merege with createVBO by passing buffer type
void Cube::createEAB(GLuint* eab, GLuint size){
	// create buffer object
	glGenBuffers(1, eab);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eab);

	// initialize buffer object
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//@todo checkGLError();

}

void Cube::deleteEAB(GLuint* eab){
	glBindBuffer(1, *eab);
	glDeleteBuffers(1, eab);

	*eab = 0;
}

void Cube::setVBOData(){
	// upload vertices
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO);
	// Prep arr
	float3* verts = (float3*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// Set eahc vert, in a horriblish way for now. 
	verts[0] = float3{ 0.0f, 0.0f, 0.0f };
	verts[1] = float3{ 0.0f, 0.0f, 1.0f };
	verts[2] = float3{ 0.0f, 1.0f, 0.0f };
	verts[3] = float3{ 0.0f, 1.0f, 1.0f };
	verts[4] = float3{ 1.0f, 0.0f, 0.0f };
	verts[5] = float3{ 1.0f, 0.0f, 1.0f };
	verts[6] = float3{ 1.0f, 1.0f, 0.0f };
	verts[7] = float3{ 1.0f, 1.0f, 1.0f };
	verts[8] = float3{ 0.0f, 0.0f, 0.0f };
	verts[9] = float3{ 0.0f, 0.0f, 1.0f };
	verts[10] = float3{ 0.0f, 1.0f, 0.0f };
	verts[11] = float3{ 0.0f, 1.0f, 1.0f };
	verts[12] = float3{ 1.0f, 0.0f, 0.0f };
	verts[13] = float3{ 1.0f, 0.0f, 1.0f };
	verts[14] = float3{ 1.0f, 1.0f, 0.0f };
	verts[15] = float3{ 1.0f, 1.0f, 1.0f };
	verts[16] = float3{ 0.0f, 0.0f, 0.0f };
	verts[17] = float3{ 0.0f, 0.0f, 1.0f };
	verts[18] = float3{ 0.0f, 1.0f, 0.0f };
	verts[19] = float3{ 0.0f, 1.0f, 1.0f };
	verts[20] = float3{ 1.0f, 0.0f, 0.0f };
	verts[21] = float3{ 1.0f, 0.0f, 1.0f };
	verts[22] = float3{ 1.0f, 1.0f, 0.0f };
	verts[23] = float3{ 1.0f, 1.0f, 1.0f };

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, this->normalVBO);
	// Prep arr
	float3* normals = (float3*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// Populate
	normals[0] = float3{ 0.0f, 0.0f, -1.0f }; // f2
	normals[1] = float3{ 0.0f, 0.0f, 1.0f }; // f0
	normals[2] = float3{ 0.0f, 0.0f, -1.0f }; // f2
	normals[3] = float3{ 0.0f, 0.0f, 1.0f }; // f0
	normals[4] = float3{ 0.0f, 0.0f, -1.0f }; // f2
	normals[5] = float3{ 0.0f, 0.0f, 1.0f }; // f0
	normals[6] = float3{ 0.0f, 0.0f, -1.0f }; // f2
	normals[7] = float3{ 0.0f, 0.0f, 1.0f }; // f0

	normals[8] = float3{ -1.0f, 0.0f, 0.0f }; // f3
	normals[9] = float3{ -1.0f, 0.0f, 0.0f }; // f3
	normals[10] = float3{ -1.0f, 0.0f, 0.0f }; // f3
	normals[11] = float3{ -1.0f, 0.0f, 0.0f }; // f3
	normals[12] = float3{ 1.0f, 0.0f, 0.0f }; // f1
	normals[13] = float3{ 1.0f, 0.0f, 0.0f }; // f1
	normals[14] = float3{ 1.0f, 0.0f, 0.0f }; // f1
	normals[15] = float3{ 1.0f, 0.0f, 0.0f }; // f1

	normals[16] = float3{ 0.0f, -1.0f, 0.0f }; // f4
	normals[17] = float3{ 0.0f, -1.0f, 0.0f }; // f4
	normals[18] = float3{ 0.0f, 1.0f, 0.0f }; // f5
	normals[19] = float3{ 0.0f, 1.0f, 0.0f }; // f5
	normals[20] = float3{ 0.0f, -1.0f, 0.0f }; // f4
	normals[21] = float3{ 0.0f, -1.0f, 0.0f }; // f4
	normals[22] = float3{ 0.0f, 1.0f, 0.0f }; // f5
	normals[23] = float3{ 0.0f, 1.0f, 0.0f }; // f5

	glUnmapBuffer(GL_ARRAY_BUFFER);

	// Upload colour data
	glBindBuffer(GL_ARRAY_BUFFER, this->colorVBO);
	// Prep arr
	color* colors = (color*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	//Populate
	colors[0] = color{ 1.0f, 0.0f, 0.0f, 1.0f }; // f2
	colors[1] = color{ 0.0f, 1.0f, 0.0f, 1.0f }; // f0
	colors[2] = color{ 1.0f, 0.0f, 0.0f, 1.0f }; // f2
	colors[3] = color{ 0.0f, 1.0f, 0.0f, 1.0f }; // f0
	colors[4] = color{ 1.0f, 0.0f, 0.0f, 1.0f }; // f2
	colors[5] = color{ 0.0f, 1.0f, 0.0f, 1.0f }; // f0
	colors[6] = color{ 1.0f, 0.0f, 0.0f, 1.0f }; // f2
	colors[7] = color{ 0.0f, 1.0f, 0.0f, 1.0f }; // f0
	colors[8] = color{ 0.0f, 0.0f, 1.0f, 1.0f }; // f3
	colors[9] = color{ 0.0f, 0.0f, 1.0f, 1.0f }; // f3
	colors[10] = color{ 1.0f, 1.0f, 1.0f, 1.0f }; // f3
	colors[11] = color{ 1.0f, 1.0f, 1.0f, 1.0f }; // f3
	colors[12] = color{ 1.0f, 1.0f, 0.0f, 1.0f }; // f1
	colors[13] = color{ 1.0f, 1.0f, 0.0f, 1.0f }; // f1
	colors[14] = color{ 1.0f, 1.0f, 0.0f, 1.0f }; // f1
	colors[15] = color{ 1.0f, 1.0f, 0.0f, 1.0f }; // f1
	colors[16] = color{ 1.0f, 0.0f, 1.0f, 1.0f }; // f4
	colors[17] = color{ 1.0f, 0.0f, 1.0f, 1.0f }; // f4
	colors[18] = color{ 0.0f, 1.0f, 1.0f, 1.0f }; // f5
	colors[19] = color{ 0.0f, 1.0f, 1.0f, 1.0f }; // f5
	colors[20] = color{ 1.0f, 0.0f, 1.0f, 1.0f }; // f4
	colors[21] = color{ 1.0f, 0.0f, 1.0f, 1.0f }; // f4
	colors[22] = color{ 0.0f, 1.0f, 1.0f, 1.0f }; // f5
	colors[23] = color{ 0.0f, 1.0f, 1.0f, 1.0f }; // f5

	glUnmapBuffer(GL_ARRAY_BUFFER);

	// define indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementArrayBuffer);
	int* indices = (int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	// This is disgusting tbh
	indices[0] = 3;
	indices[1] = 1;
	indices[2] = 5;
	indices[3] = 3;
	indices[4] = 5;
	indices[5] = 7;
	indices[6] = 15;
	indices[7] = 13;
	indices[8] = 12;
	indices[9] = 15;
	indices[10] = 12;
	indices[11] = 14;
	indices[12] = 6;
	indices[13] = 4;
	indices[14] = 0;
	indices[15] = 6;
	indices[16] = 0;
	indices[17] = 2;
	indices[18] = 10;
	indices[19] = 8;
	indices[20] = 9;
	indices[21] = 10;
	indices[22] = 9;
	indices[23] = 11;
	indices[24] = 16;
	indices[25] = 20;
	indices[26] = 21;
	indices[27] = 16;
	indices[28] = 21;
	indices[29] = 17;
	indices[20] = 22;
	indices[31] = 18;
	indices[32] = 19;
	indices[33] = 22;
	indices[34] = 19;
	indices[35] = 23;


	// @todo
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void Cube::renderVBO(){
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->normalVBO);
	glNormalPointer(GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->colorVBO);
	glColorPointer(4, GL_FLOAT, 0, 0);

	glDrawElements(GL_TRIANGLES, NUM_TRIANGLES * VERTICES_PER_TRIANGLE, GL_UNSIGNED_INT, 0); 
	// glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}