#include "HUD.h"

#include "Overlay.h"
#include <glm/gtc/matrix_transform.inl>
#include <glm/gtc/type_ptr.hpp>

/*
Creates a new HUD, specifying the window dimensions
@param width Window width
@param height Window height
@note This is done within Visualisation, regular users have no reason to instaniate a HUD
*/
HUD::HUD(unsigned int width, unsigned int height)
	: modelViewMat()
	, projectionMat()
	, width(width)
	, height(height)
{
	resizeWindow(width, height);
}
/*
Adds an overlay element to the HUD
@param overlay The overlay element to be rendered as part of the HUD
@param anchorV Vertical Anchoring location (North|Center|South), defaults Center
@param anchordH Horizontal Anchoring location (East|Center|West), defaults Center
@param x The horizontal offset from the anchored position
@param y The vertical offset from the anchored position
@param z-index The priority for which overlay should be on-top.
@note If two items share the same z-index, the new item will insert as though it has the lower z-index (and be rendered underneath)
@note Adding the same overlay to HUD a second time, will remove it's first instance.
*/
void HUD::add(std::shared_ptr<Overlay> overlay, AnchorV anchorV, AnchorH anchorH, int x, int y, int zIndex)
{
    remove(overlay);
	std::list<std::shared_ptr<Item>>::iterator it = stack.begin();
	for (; it != stack.end(); ++it)
	{
		//Find first element with a lower z-index
		if ((*it)->zIndex < zIndex)
		{
			break;
		}
	}
	
	std::list<std::shared_ptr<Item>>::iterator item = stack.insert(it, std::make_shared<Item>(overlay, x, y, this->width, this->height, anchorV, anchorH, zIndex));
	overlay->setHUDItem(*item);
}
/*
Removes the specified overlay from the HUD
@return The number of overlays removed
*/
unsigned int HUD::remove(std::shared_ptr<Overlay> overlay)
{
	unsigned int removed = 0;
	std::list<std::shared_ptr<Item>>::iterator it = stack.begin();
	while(it != stack.end())
	{
		if((*it)->overlay==overlay)
		{
			stack.erase(it++);
			removed++;
		}
		else
			++it;
	}
	return removed;
}
/*
Removes all overlays from the HUD
*/
void HUD::clear()
{
	stack.clear();
}
/*
Returns the number of overlays currently on the HUD
@return The number of overlays
*/
unsigned int HUD::getCount()
{
	return (unsigned int)stack.size();
}
/*
Calls reload on all held overlay elements
@note Some overlay subclasses may not implement reload, however their shaders will be reloaded
*/
void HUD::reload()
{
	for (std::list<std::shared_ptr<Item>>::iterator it = stack.begin(); it != stack.end(); ++it)
		(*it)->overlay->_reload();
}
/*
Renders all HUD elements in reverse z-index order, with GL_DEPTH_TEST disabled
*/
void HUD::render()
{
    GL_CALL(glDisable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
	//Iterate stack from lowest z-index to highest
	std::list<std::shared_ptr<Item>>::reverse_iterator it = stack.rbegin();
	while (it != stack.rend())
	{
		(*it)->overlay->render(&modelViewMat, &projectionMat, (*it)->fvbo);
		++it;
    }
    GL_CALL(glDisable(GL_BLEND));
    GL_CALL(glEnable(GL_DEPTH_TEST));
}
/*
Repositions all HUD ovlerays according to the new window dimensions and their repsective anchoring/offset args
@param w New window width
@param h New window height
*/
void HUD::resizeWindow(const unsigned int w, const unsigned int h)
{
	this->width = w;
	this->height = h;
	//Camera at origin looking down y axis, with up vector looking up z axis
	//Top left is origin, bottom right is (width, -height)
	//Bottom left is origin
	//Top right is (width,height)
	//Rendering the z plane 0 to -1
	projectionMat =
		glm::ortho<float>(
			0.0f, (float)this->width,
			0.0f, (float)this->height,
			0.0f, 1.0f
			);
	for (std::list<std::shared_ptr<Item>>::iterator it = stack.begin(); it != stack.end(); ++it)
		(*it)->resizeWindow(this->width, this->height);
}
/*
Initialises the HUDItem, by calculating the elements position
@param overlay The overlay element to be rendered as part of the HUD
@param x The horizontal offset from the anchored position
@param y The vertical offset from the anchored position
@param window_w The width of the HUD
@param windows_h The height of the HUD
@param anchorV Vertical Anchoring location (North|Center|South), defaults Center
@param anchordH Horizontal Anchoring location (East|Center|West), defaults Center
@param z-index The priority for which overlay should be on-top.
*/
HUD::Item::Item(std::shared_ptr<Overlay> overlay, int x, int y, unsigned int window_w, unsigned int window_h, AnchorV anchorV, AnchorH anchorH, int zIndex)
	: overlay(overlay)
	, x(x)
	, y(y)
	, anchorV(anchorV)
	, anchorH(anchorH)
	, zIndex(zIndex)
	, vbo(0)
	, data(nullptr)
{
	//Init vbo's
	unsigned int bufferSize=0;
	bufferSize += 4 * sizeof(glm::vec3); //4 points to a quad
	bufferSize += 4 * sizeof(glm::vec2);//4 points to a tex coord
	data = malloc(bufferSize);
	//Setup tex coords
	glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(data)+4));
	texCoords[0] = glm::vec2(0.0f, 1.0f); //TopLeft
	texCoords[1] = glm::vec2(0.0f, 0.0f); //BottomLeft
	texCoords[2] = glm::vec2(1.0f, 1.0f); //TopRight
	texCoords[3] = glm::vec2(1.0f, 0.0f); //BottomRight
	//Initalise buffer
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, data, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Setup vertices
	resizeWindow(window_w, window_h);
	//Link Vertex Attributes TO SHADER??!?!??!?
	Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
	pos.vbo = vbo;
	pos.count = 4;
	pos.data = data;
	pos.offset = 0;
	pos.stride = 0;
	overlay->getShaders()->setPositionsAttributeDetail(pos);
	Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
	texCo.vbo = vbo;
	texCo.count = 4;
    texCo.data = texCoords;
	texCo.offset = 4*sizeof(glm::vec3);
	texCo.stride = 0;
	overlay->getShaders()->setTexCoordsAttributeDetail(texCo);
	//Setup faces
	GL_CALL(glGenBuffers(1, &fvbo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
	const int faces[] = { 0, 1, 2, 3 };
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(int), &faces, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
HUD::Item::~Item()
{
    GL_CALL(glDeleteBuffers(1, &vbo));
    GL_CALL(glDeleteBuffers(1, &fvbo));
    free(data);
}
/*
Update the overlays quad location, based on new window size, anchors, offsets and overlay dimensions
@param w The new window width
@param h The new window height
*/
void HUD::Item::resizeWindow(const unsigned int w, const unsigned int h)
{
	//Track parameters, so when called from overlay we can reuse previous
	static unsigned int width, height;
	if (w>0 && h>0)
	{
		width = w;
		height = h;
	}
	const float depth = -0.5f;
	glm::vec3 *topLeft		= static_cast<glm::vec3*>(data);
	glm::vec3 *bottomLeft   = topLeft + 1;
	glm::vec3 *topRight     = bottomLeft + 1;
	glm::vec3 *bottomRight  = topRight + 1;
	*bottomLeft = glm::vec3(0, 0, depth);
	//Anchor horizontal
	if (anchorH == AnchorH::West)
		bottomLeft->x = 0;
	else if (anchorH == AnchorH::Center)
		bottomLeft->x = floor((width / 2.0f) - (overlay->getWidth() / 2.0f));//Cast back to int to prevent tearing
	else if (anchorH == AnchorH::East)
		bottomLeft->x = static_cast<float>(width - overlay->getWidth());
	//Anchor vertical
	if (anchorV == AnchorV::South)
		bottomLeft->y = 0;
	else if (anchorV==AnchorV::Center)
		bottomLeft->y = floor((height / 2.0f) - (overlay->getHeight() / 2.0f));//Cast back to int to prevent tearing
	else if (anchorV==AnchorV::North)
		bottomLeft->y = static_cast<float>(height - overlay->getHeight());
	//Apply offsets
    bottomLeft->x += x;
    bottomLeft->y += y;
	//Adjust other corners relative to topLeft & overlay size
	*topLeft     = glm::vec3(*bottomLeft);
	*bottomRight = glm::vec3(*bottomLeft);
	*topRight    = glm::vec3(*bottomLeft);
	topLeft->y	   += overlay->getHeight();
	bottomRight->x += overlay->getWidth();
	topRight->x    += overlay->getWidth();
	topRight->y    += overlay->getHeight();

	//update data within vbo
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), data));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    //If required, pass to shader
    auto pair = ShaderCore::findUniform("_viewportDims", overlay->getShaders()->getProgram());
    if (std::get<0>(pair) != -1)
    {
        glm::ivec2 viewportDims(width, height);
        overlay->getShaders()->addStaticUniform("_viewportDims", glm::value_ptr(viewportDims), 2);
    }
}