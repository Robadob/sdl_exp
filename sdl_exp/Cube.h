#pragma once

#include <GL/glew.h>
#include "SDL/SDL_opengl.h"

class Cube
{
public:
	Cube(double scale = 1.0);
	~Cube();

	void render();

	void createVBO(GLuint* vbo, GLuint size);
	void deleteVBO(GLuint* vbo);
	void createEAB(GLuint* eab, GLuint size);
	void deleteEAB(GLuint* eab);
	void setVBOData();
	void renderVBO();

private:
	double scale;
	GLuint vertexVBO;
	GLuint normalVBO;
	GLuint colorVBO;
	GLuint elementArrayBuffer;
};

