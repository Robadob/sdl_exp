#include "Axis.h"

#include <stdio.h>
#include "SDL/SDL_opengl.h"

Axis::Axis(float length){
    this->length = length;
}


Axis::~Axis()
{
}


void Axis::render(){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPushMatrix();
        glBegin(GL_LINES);
            // X axis
            glColor3f(1.0, 0.0, 0.0);     // Red
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(this->length, 0.0, 0.0);

            // Y axis
            glColor3f(0.0, 1.0, 0.0);     // Green
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, this->length, 0.0);

            // Z axis
            glColor3f(0.0, 0.0, 1.0);     // Blue
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, 0.0, this->length);
        glEnd();
    glPopMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
