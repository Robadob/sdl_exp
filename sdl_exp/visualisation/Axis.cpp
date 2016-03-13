#include "Axis.h"

#include "GLcheck.h"
/*
Constructs a new axis object
@param length The length of each drawn axis
*/
Axis::Axis(float length){
    this->length = length;
}
/*
Default destructor
*/
Axis::~Axis(){
}
/*
Renders a simple axis marker. Red displays the positive x, Green the positive y and Blue the positive z.
*/
void Axis::render(){
    glUseProgram(0); //Use default shader
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glPushAttrib(GL_ENABLE_BIT);
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
    GL_CHECK();
    glPopAttrib();
    glPopMatrix();
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}
