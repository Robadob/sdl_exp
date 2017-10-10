#include "Overlay.h"
#include <glm/gtc/type_ptr.hpp>

/*
Updates the overlays width and triggers HUD::Item.resizeWindow() if available.
@param w The new overlay width
*/
void Overlay::setWidth(unsigned int w)
{
	if (w == width)
		return;
	width = w;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}
/*
Updates the overlays width and triggers HUD::Item.resizeWindow() if available.
@param h The new overlay height
*/
void Overlay::setHeight(unsigned int h)
{
	if (h == height)
		return;
	height = h;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}
/*
Updates the overlays width and triggers HUD::Item.resizeWindow() if available.
@param w The new overlay width
@param h The new overlay height
*/
void Overlay::setDimensions(unsigned int w, unsigned int h)
{
	if (w == width&&h == height)
		return;
	width = w;
	height = h;
	if (auto t = hudItem.lock())
		t->resizeWindow();
}
/*
Sets the HUD::item attatched to the overlay, so that resize events can be triggered
*/
void Overlay::setHUDItem(std::shared_ptr<HUD::Item> ptr)
{
    hudItem = std::weak_ptr<HUD::Item>(ptr);
    if (auto t = hudItem.lock())
    {//Process any stored flips
        if (flippedV)
            t->flipVertical();
        if (flippedH)
            t->flipHorizontal();
    }
}
/*
Creates a new overlay, this is an abstract class and should not be directly instantiated
@param shaders Shared pointer to the shaders object to be used when rendering the overlay
@param width The width of the overlay
@param height The height of the overlay
@note If the dimensions of the overlay are not known at initialisation, call setDimensions() as soon as they are known.
*/
Overlay::Overlay(std::shared_ptr<Shaders> shaders, unsigned int width, unsigned int height)
	: hudItem()
    , visible(true)
    , shaders(shaders)
    , width(width)
    , height(height)
    , flippedV(false)
    , flippedH(false)
{
}
/*
Renders the overlay using the provided details
@param mv The modelview matrix
@param proj The projection matrix
@param fbo The buffer object holding the face indices
*/
void Overlay::render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint fbo)
{
    if (!visible)
		return;
	if (this->shaders != nullptr)
	{
		shaders->setViewMatPtr(mv);
		shaders->setProjectionMatPtr(proj);
		shaders->useProgram();
	}
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
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
	glPushMatrix(); 
	    GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0));
	glPopMatrix();
    //GL_CALL(glDisable(GL_BLEND));
	if (this->shaders != nullptr)
		shaders->clearProgram();
	//Do something, setup projection/quad
}
/*
Sets whether the overlay should be rendered or not
@param isVisible True if the overlay should be rendered
*/
void Overlay::setVisible(bool isVisible)
{
    this->visible = isVisible;
}
void Overlay::flipVertical()
{
    flippedV = !flippedV;
    if (auto t = hudItem.lock())
        t->flipVertical();
}
void Overlay::flipHorizontal()
{
    flippedH = !flippedH;
    if (auto t = hudItem.lock())
        t->flipHorizontal();
}