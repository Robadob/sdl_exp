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
