#include "Lines.h"

#define RESIZE_FACTOR 2
#define INITIAL_MAX_COUNT 32

const unsigned int Lines::LINE_SIZE = sizeof(float) * 7 * 2;
const unsigned int Lines::LINE_INDICES_SIZE = sizeof(unsigned short) * 2;
const unsigned int Lines::VERTEX_STRIDE = 7;
const unsigned int Lines::VERTEX_A_OFFSET = 0;
const unsigned int Lines::VERTEX_B_OFFSET = 7;
const unsigned int Lines::COLOR_A_OFFSET = 3;
const unsigned int Lines::COLOR_B_OFFSET = 10;

Lines::Lines(const std::vector<Line> &lines)
	: vertColor(nullptr)
	, indices(nullptr)
	, maxCount(INITIAL_MAX_COUNT)
	, vbo(0)
	, fvbo(0)
	, vertices(GL_FLOAT, 3, sizeof(float))
	, colors(GL_FLOAT, 4, sizeof(float))
	, faces(GL_UNSIGNED_SHORT, 2, sizeof(unsigned short))
	, shaders(std::make_shared<Shaders>(Stock::Shaders::COLOR))
{
	//Create buffers
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glGenBuffers(1, &fvbo));
	count = (unsigned int)lines.size();
	//Setup maxCount
	while (count>maxCount)
		maxCount *= RESIZE_FACTOR;
	//Allocate host memory
	vertColor = static_cast<float*>(malloc(maxCount * LINE_SIZE));
	indices = static_cast<unsigned short*>(malloc(maxCount * LINE_INDICES_SIZE));
	//Fill host memory
	unsigned int offset = 0;
	for (auto &&it = lines.begin(); it != lines.end(); ++it)
	{
		memcpy(&vertColor[offset + VERTEX_A_OFFSET], &it->vertices[0], sizeof(glm::vec3));
		memcpy(&vertColor[offset + VERTEX_B_OFFSET], &it->vertices[1], sizeof(glm::vec3));
		memcpy(&vertColor[offset + COLOR_A_OFFSET], &it->colors[0], sizeof(glm::vec4));
		memcpy(&vertColor[offset + COLOR_B_OFFSET], &it->colors[1], sizeof(glm::vec4));
		offset += VERTEX_STRIDE;
	}
	for (unsigned short i = 0; i < maxCount * 2; ++i)
		indices[i]=i;
	//auto *vc = (glm::vec3*)vertColor;
	//	vc[0] = glm::vec3(0, 0, 0);
	//	vc[1] = glm::vec3(-25, -25, -25);
	//	vc[2] = glm::vec3(-25, 25, 0);
	//	vc[3] = glm::vec3(0, 0, 25);
	//	vc[4] = glm::vec3(0, 0, 25);
	//	vc[5] = glm::vec3(25, 0, 0);
	//Fill device buffers
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, maxCount * LINE_SIZE, &vertColor, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxCount * LINE_INDICES_SIZE, &indices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	//Configure shaders
	vertices.count = count*2;
	vertices.data = (void*)&vertColor;
	vertices.vbo = vbo;
	vertices.stride = 4 * sizeof(float);

	colors.count = count * 2;
	colors.data = (void*)&vertColor;
	colors.offset = 3 * sizeof(float);
	colors.vbo = vbo;
	colors.stride = 3 * sizeof(float);

	faces.count = count;
	faces.data = (void*)&indices;
	faces.vbo = fvbo;

	shaders->setPositionsAttributeDetail(vertices);
	shaders->setColorsAttributeDetail(colors);
}
Lines::Lines(const Line &line)
	: Lines(std::vector<Line>({ line }))
{ }
Lines::Lines(const std::initializer_list<Point> &pointPairs)
	: Lines(makeInitList(pointPairs))
{ }
Lines::Lines(const Point &vertexA, const Point &vertexB)
: Lines({ vertexA, vertexB })
{ }
Lines::Lines(const glm::vec3 &vertexA, const glm::vec3 &vertexB, const glm::vec4 &color)
	: Lines(Line({ vertexA, color }, { vertexB, color }))
{ }
Lines::Lines(const glm::vec3 &vertexA, const glm::vec3 &vertexB, const glm::vec3 &color)
	: Lines({ vertexA, vertexB, glm::vec4(color, 1.0f) })
{ }
Lines::Lines(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color)
	: Lines(makeInitList(vertices, color))
{ }
Lines::Lines(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color)
	: Lines(makeInitList(vertices, color))
{ }

std::vector<Line> Lines::makeInitList(const std::initializer_list<Point> &pointPairs)
{
	assert(pointPairs.size() % 2 == 0);//Must have even number of points, this isn't a polyline
	std::vector<Line> rtn(pointPairs.size() / 2);
	unsigned int i = 0;
	for (auto &&it = pointPairs.begin(); it != pointPairs.end(); it+=2)
	{
		rtn[i++] = { *it, *(it + 1) };
	}
	return rtn;
}
std::vector<Line> Lines::makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec4 &color)
{
	assert(vertices.size() % 2 == 0);//Must have even number of points, this isn't a polyline
	std::vector<Line> rtn(vertices.size() / 2);
	unsigned int i = 0;
	for (auto &&it = vertices.begin(); it != vertices.end(); it += 2)
	{
		rtn[i++] = { { *it, color }, { *(it + 1), color } };
	}
	return rtn;
}
std::vector<Line> Lines::makeInitList(const std::initializer_list<glm::vec3> vertices, const glm::vec3 &color)
{
	return makeInitList(vertices, glm::vec4(color, 1.0f));
}

Lines::~Lines()
{
	//Free host memory
	if (vertColor)
		free(vertColor);
	if (indices)
		free(indices);
	//Delete buffers
	GL_CALL(glDeleteBuffers(1, &vbo));
	GL_CALL(glDeleteBuffers(1, &fvbo));
}

unsigned int Lines::addLine(const Line &line)
{
	if (count==maxCount)
	{
		//Resize host memory
		maxCount *= RESIZE_FACTOR;
		float *t = (float*)malloc(maxCount*LINE_SIZE);
		memcpy(t, vertColor, sizeof(count*LINE_SIZE));
		free(vertColor);
		vertColor = t;
		//Extend face vbo
		for (unsigned short i = count * 2; i < maxCount * 2; ++i)
			indices[i]=i;
		//Add Line
		auto offset = count++*VERTEX_STRIDE;
		memcpy(&vertColor[offset + VERTEX_A_OFFSET], &line.vertices[0], sizeof(glm::vec3));
		memcpy(&vertColor[offset + VERTEX_B_OFFSET], &line.vertices[1], sizeof(glm::vec3));
		memcpy(&vertColor[offset + COLOR_A_OFFSET], &line.colors[0], sizeof(glm::vec4));
		memcpy(&vertColor[offset + COLOR_B_OFFSET], &line.colors[1], sizeof(glm::vec4));
		//Resize device buffers
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, maxCount * LINE_SIZE, &vertColor, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
		GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxCount * LINE_INDICES_SIZE, &faces, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
	else
	{
		setLine(count++, line);
	}
	//Update Vertex Attribute Data
	vertices.count = count * 2;
	colors.count = count * 2;
	faces.count = count;
	return count - 1;
}
unsigned int Lines::addLine(const Point &pointA, const Point &pointB)
{
	return addLine({ pointA, pointB });
}
unsigned int Lines::addLine(const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec4 color)
{
	return addLine({ { vertexA, color }, { vertexB, color } });
}
unsigned int Lines::addLine(const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec3 color)
{
	return addLine({ { vertexA, glm::vec4(color, 1.0f) }, { vertexB, glm::vec4(color, 1.0f) } });
}
bool Lines::setLine(const unsigned int &index, const Line &line)
{
	if (index >= this->count)
		return false;
	auto offset = index*VERTEX_STRIDE;
	memcpy(&vertColor[offset + VERTEX_A_OFFSET], &line.vertices[0], sizeof(glm::vec3));
	memcpy(&vertColor[offset + VERTEX_B_OFFSET], &line.vertices[1], sizeof(glm::vec3));
	memcpy(&vertColor[offset + COLOR_A_OFFSET],  &line.colors[0],   sizeof(glm::vec4));
	memcpy(&vertColor[offset + COLOR_B_OFFSET],  &line.colors[1],   sizeof(glm::vec4));
	//Add to device buffer
	return true;
}
bool Lines::setLine(const unsigned int &index, const Point &pointA, Point &pointB)
{
	return setLine(index, { pointA, pointB });
}
bool Lines::setLine(const unsigned int &index, const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec4 color)
{
	return setLine(index, { { vertexA, color }, { vertexB, color } });
}
bool Lines::setLine(const unsigned int &index, const glm::vec3 &vertexA, const glm::vec3 &vertexB, glm::vec3 color)
{
	return setLine(index, { { vertexA, glm::vec4(color, 1.0f) }, { vertexB, glm::vec4(color, 1.0f) } });
}

glm::mat4 Lines::render(const unsigned int &shaderIndex, glm::mat4 transform)
{
#ifdef _DEBUG
	assert(shaderIndex == 0);
#endif
	shaders->useProgram();
	transform = shaders->overrideModelMat(&transform);
	//GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, static_cast<char *>(nullptr) + 0));// this->positions.offset));
	//GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, static_cast<char *>(nullptr) + 0));//this->colors.offset));

	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));
	//GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	GL_CALL(glDrawElements(GL_LINES, faces.count * faces.components, faces.componentType, nullptr));
	//GL_CALL(glDrawElements(GL_LINES, 1 * faces.components, faces.componentType, nullptr));
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	//GL_CALL(glDisable(GL_BLEND));
	shaders->clearProgram();
	return transform;
}
void Lines::reload()
{
	shaders->reload();
}
void Lines::setViewMatPtr(glm::mat4 const *viewMat)
{
	shaders->setViewMatPtr(viewMat);
}
void Lines::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
	shaders->setProjectionMatPtr(projectionMat);
}