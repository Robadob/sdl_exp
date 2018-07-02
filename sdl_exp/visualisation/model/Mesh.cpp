#include "Mesh.h"
#include "Model.h"
#include <glm/mat4x4.hpp>

void Mesh::render(glm::mat4 &transform, const unsigned int &shaderIndex) const
{
	if (!visible)
		return;
	data->materials[materialIndex]->use(transform, shaderIndex, false);
	//Render
	GL_CALL(glDrawElements(faceType, faceSize, GL_UNSIGNED_INT, (void *)(((char *)0)+byteOffset)));//Don't cast a 4 byte int to 8 byte pointer, increment the point by the number of bytes
}
BoundingBox3D Mesh::calculateBoundingBox(glm::mat4 transform) const
{
    BoundingBox3D rtn;
    for (unsigned int i = 0; i < faceSize; ++i)
    {
        rtn.include(data->vertices[data->faces[byteOffset / sizeof(unsigned int) + i]]);
    }
    return rtn;
}