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
	
	std::list<std::shared_ptr<Item>>::iterator item = stack.insert(it, std::make_shared<Item>(overlay, x, y, anchorV, anchorH, zIndex));
	(*item)->resizeWindow(width, height);
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
	return stack.size();
}
/*
Calls reload on all held overlay elements
*/
void HUD::reload()
{
	for (std::list<Item>::iterator it = stack.begin(); it != stack.end(); ++it)
		(*it).overlay->reload();
}
void HUD::render()
{
	//Iterate stack from lowest z-index to highest
	std::list<Item>::iterator it = stack.end();
	do
	{
		(*it).overlay->render(&modelViewMat, &projectionMat, (*it).vbo);
		--it;
	} while (it != stack.begin());
}
void HUD::resizeWindow(const unsigned int w, const unsigned int h)
{
	this->width = w;
	this->height = h;
	//Camera at origin looking down y axis, with up vector looking up z axis
	projectionMat = 
		glm::lookAt<float>(
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(0, 0, 1)
		) 
		* 
		glm::ortho<float>(
			0.0f, this->width,
			0.0f, 1.0f, 
			-this->height, 0.0f
		);
	for (std::list<Item>::iterator it = stack.begin(); it != stack.end(); ++it)
		(*it).resizeWindow(this->width, this->height);
}

HUD::Item::Item(std::shared_ptr<Overlay> overlay, int x, int y, HUDAnchorV anchorV, HUDAnchorH anchorH, int zIndex)
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
	texCoords[0] = glm::vec2(0.0f, 0.0f);
	texCoords[1] = glm::vec2(0.0f, 1.0f);
	texCoords[2] = glm::vec2(1.0f, 1.0f);
	texCoords[3] = glm::vec2(1.0f, 0.0f);
	//Initalise buffer
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, data, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	//Setup vertices
	resizeWindow(overlay->getWidth(), overlay->getHeight());
	//Link Vertex Attributes TO SHADER??!?!??!?
	Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
	pos.vbo = vbo;
	pos.count = 4;
	pos.data = data;
	pos.offset = 0;
	pos.stride = 0;
	overlay->getShaders()->setPositionsAttributeDetail(pos);
	Shaders::VertexAttributeDetail texCo(GL_FLOAT, 3, sizeof(float));
	texCo.vbo = vbo;
	texCo.count = 4;
	texCo.data = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(data)+4));
	texCo.offset = 4*sizeof(glm::vec3);
	texCo.stride = 0;
	overlay->getShaders()->setTexCoordsAttributeDetail(texCo);
	//Setup faces
	GL_CALL(glGenBuffers(1, &fvbo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
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
	const float depth = 0.5f;
	glm::vec3 *topLeft		= static_cast<glm::vec3*>(data);
	glm::vec3 *bottomLeft	= topLeft + 1;
	glm::vec3 *bottomRight	= bottomLeft + 1;
	glm::vec3 *topRight		= bottomRight + 1;
	*topLeft =		glm::vec3(0, depth, 0);
	//Anchor horizontal
	if (anchorH == HUDAnchorH::West)
		topLeft->x = 0;
	else if (anchorH == HUDAnchorH::Center)
		topLeft->x = (int)((width / 2.0f) - (overlay->getWidth() / 2.0f));//Cast back to int to prevent tearing
	else if (anchorH == HUDAnchorH::East)
		topLeft->x = width - overlay->getWidth();
	//Anchor vertical
	if (anchorV == HUDAnchorV::North)
		topLeft->z = 0;
	else if (anchorV==HUDAnchorV::Center)
		topLeft->z = (int)-((height / 2) - (overlay->getHeight() / 2));//Cast back to int to prevent tearing
	else if (anchorV==HUDAnchorV::South)
		topLeft->z = -(height - overlay->getHeight());
	//Adjust other corners relative to topLeft & overlay size
	*bottomLeft	 = glm::vec3(topLeft->x, depth, topLeft->z);
	*bottomRight = glm::vec3(topLeft->x, depth, topLeft->z);
	*topRight    = glm::vec3(topLeft->x, depth, topLeft->z);
	bottomLeft->z  -= overlay->getHeight();
	bottomRight->x += overlay->getWidth();
	bottomRight->z -= overlay->getHeight();
	topRight->x    += overlay->getWidth();
	//update data within vbo
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), data));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}