#include "HUD.h"

#include "Overlay.h"
#include <glm/gtc/matrix_transform.inl>

HUD::HUD(unsigned int width, unsigned int height)
	: modelViewMat()
	, projectionMat()
	, width(width)
	, height(height)
{
	resizeWindow(width, height);
}
void HUD::add(std::shared_ptr<Overlay> overlay, int x, int y, HUDAnchorV anchorV, HUDAnchorH anchorH, int zIndex)
{
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
unsigned int HUD::removeAll(std::shared_ptr<Overlay> overlay)
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
void HUD::clear()
{
	stack.clear();
}
unsigned int HUD::getCount()
{
	return (unsigned int)stack.size();
}
/*
Calls reload on all held overlay elements
*/
void HUD::reload()
{
	for (std::list<std::shared_ptr<Item>>::iterator it = stack.begin(); it != stack.end(); ++it)
		(*it)->overlay->_reload();
}
void HUD::render()
{
	//Iterate stack from lowest z-index to highest
	std::list<std::shared_ptr<Item>>::reverse_iterator it = stack.rbegin();
	while (it != stack.rend())
	{
		(*it)->overlay->render(&modelViewMat, &projectionMat, (*it)->fvbo);
		++it;
	}
}
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

HUD::Item::Item(std::shared_ptr<Overlay> overlay, int x, int y, unsigned int window_w, unsigned int window_h, HUDAnchorV anchorV, HUDAnchorH anchorH, int zIndex)
	: overlay(overlay)
	, x(x)
	, y(y)
	, anchorV(anchorV)
	, anchorH(anchorH)
	, zIndex(zIndex)
	, vbo(0)
	, data(0)
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
/*Update the overlays quad, based on new window size, anchors and overlay dimensions*/
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
	if (anchorH == HUDAnchorH::West)
		bottomLeft->x = 0;
	else if (anchorH == HUDAnchorH::Center)
		bottomLeft->x = (float)(int)((width / 2.0f) - (overlay->getWidth() / 2.0f));//Cast back to int to prevent tearing
	else if (anchorH == HUDAnchorH::East)
		bottomLeft->x = (float)width - (float)overlay->getWidth();
	//Anchor vertical
	if (anchorV == HUDAnchorV::South)
		bottomLeft->y = 0;
	else if (anchorV==HUDAnchorV::Center)
		bottomLeft->y = (float)(int)((height / 2.0f) - (overlay->getHeight() / 2.0f));//Cast back to int to prevent tearing
	else if (anchorV==HUDAnchorV::North)
		bottomLeft->y = ((float)height - (float)overlay->getHeight());
	//Apply offsets
	topLeft->x += x;
	topLeft->y += y;
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
}