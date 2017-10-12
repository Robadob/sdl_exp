#include "Axis.h"
#include "util/GLcheck.h"
/*
Constructs a new axis object
@param length The length of each drawn axis
*/
Axis::Axis(float length)
	: vbo(0)
    , fvbo(0)
    , vertices(GL_FLOAT, 3, sizeof(float))
	, colors(GL_FLOAT, 3, sizeof(float))
	, faces(GL_UNSIGNED_INT, 2, sizeof(unsigned int))
	, shaders(std::make_shared<Shaders>(Stock::Shaders::COLOR))
{
	const float VERTICES_COLORS[36] = {
		0.0, 0.0, 0.0,
		length, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, length, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, length,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0
	};
	const unsigned int FACE_INDICES[6] = {
		0, 1,
		2, 3,
		4, 5
	};
	unsigned int vboSize = 36*sizeof(float);
	unsigned int fvboSize = 6 * sizeof(unsigned int);
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboSize, &VERTICES_COLORS[0], GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CALL(glGenBuffers(1, &fvbo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, fvboSize, &FACE_INDICES[0], GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	vertices.count = 6;
	vertices.data = (void*)&VERTICES_COLORS[0];
	vertices.vbo = vbo;

	colors.count = 8;
	colors.data = (void*)&VERTICES_COLORS[0];
	colors.offset = 18 * sizeof(float);
	colors.vbo = vbo;

	faces.count = 3;
	faces.data = (void*)&FACE_INDICES[0];
	faces.vbo = fvbo;

	shaders->setPositionsAttributeDetail(vertices);
	shaders->setColorsAttributeDetail(colors);
}
/*
Default destructor
*/
Axis::~Axis()
{
	GL_CALL(glDeleteBuffers(1, &vbo));
	GL_CALL(glDeleteBuffers(1, &fvbo));
}
glm::mat4 Axis::render(const unsigned int &shaderIndex, glm::mat4 transform)
{
#ifdef _DEBUG
	assert(shaderIndex == 0);
#endif
	shaders->useProgram();
	transform = shaders->overrideModelMat(&transform);
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces.vbo));
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	GL_CALL(glDrawElements(GL_LINES, faces.count * faces.components, faces.componentType, 0));
	GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	shaders->clearProgram();
	return transform;
}
void Axis::reload()
{
	shaders->reload();
}
void Axis::setViewMatPtr(glm::mat4 const *viewMat)
{
	shaders->setViewMatPtr(viewMat);
}
void Axis::setProjectionMatPtr(glm::mat4 const *projectionMat)
{
	shaders->setProjectionMatPtr(projectionMat);
}