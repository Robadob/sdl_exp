#include "Overlay.h"
#include <glm/gtc/type_ptr.hpp>

void Overlay::setWidth(unsigned int w)
{
	if (w == width)
		return;
	width = w;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}
void Overlay::setHeight(unsigned int h)
{
	if (h == height)
		return;
	height = h;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}
void Overlay::setDimensions(unsigned int w, unsigned int h)
{
	if (w == width&&h == height)
		return;
	width = w;
	height = h;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}

void Overlay::setHUDItem(std::shared_ptr<HUD::Item> ptr)
{
	hudItem = std::weak_ptr<HUD::Item>(ptr);
}

Overlay::Overlay(std::shared_ptr<Shaders> shaders, unsigned int width, unsigned int height)
	:
shaders(shaders), width(width), height(height), hudItem()
{
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
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	//Bind the faces to be rendered
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo));

	glPushMatrix(); 
	GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0));
	glPopMatrix();
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	if (this->shaders != nullptr)
		shaders->clearProgram();
	//Do something, setup projection/quad
}
