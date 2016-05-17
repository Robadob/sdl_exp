#include "Overlay.h"
#include <glm/gtc/type_ptr.hpp>

void Overlay::setWidth(unsigned int w)
{
	width = w;
}
void Overlay::setHeight(unsigned int h)
{
	height = h;
}
void Overlay::setDimensions(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
}
void Overlay::render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint vbo)
{
	if (this->shaders != nullptr)
		shaders->useProgram(mv, proj);
	else//Lets try fixed function, but no promises
	{
		//Update Projection Matrix
		glMatrixMode(GL_PROJECTION);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*proj)));
		//Update Model View Matrix
		glMatrixMode(GL_MODELVIEW);
		GL_CALL(glLoadMatrixf(glm::value_ptr(*mv)));
	}
	//Bind the faces to be rendered
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo));

	glPushMatrix();
	GL_CALL(glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0));
	glPopMatrix();
	if (this->shaders != nullptr)
		shaders->clearProgram();
	//Do something, setup projection/quad
}
