#include "Cube.h"
#include <stdio.h>
#include "SDL/SDL_opengl.h"

Cube::Cube(double scale){
	this->scale = scale;
}


Cube::~Cube(){
}

void Cube::render(){
	glPushMatrix();
		glScaled(this->scale, this->scale, this->scale);
		glBegin(GL_QUADS);


			// Begin drawing the color cube with 6 quads
			// Top face (y = 1.0f)
			// Define vertices in counter-clockwise (CCW) order with normal pointing out
			glColor3f(0.0f, 1.0f, 0.0f);     // Green
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, 1.0f);

			// Bottom face (y = -1.0f)
			glColor3f(1.0f, 0.5f, 0.0f);     // Orange
			glVertex3f(1.0f, -1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);

			// Front face  (z = 1.0f)
			glColor3f(1.0f, 0.0f, 0.0f);     // Red
			glVertex3f(1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);
			glVertex3f(1.0f, -1.0f, 1.0f);

			// Back face (z = -1.0f)
			glColor3f(1.0f, 1.0f, 0.0f);     // Yellow
			glVertex3f(1.0f, -1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);

			// Left face (x = -1.0f)
			glColor3f(0.0f, 0.0f, 1.0f);     // Blue
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);

			// Right face (x = 1.0f)
			glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(1.0f, 1.0f, 1.0f);
			glVertex3f(1.0f, -1.0f, 1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
		glEnd();  // End of drawing color-cube
	glPopMatrix();
}
