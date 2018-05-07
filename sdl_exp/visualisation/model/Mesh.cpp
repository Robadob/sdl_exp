#include "Mesh.h"
#include "Model.h"
#include <glm/mat4x4.hpp>

void Mesh::render(glm::mat4 &transform) const
{
	if (!visible)
		return;
    data->materials[materialIndex]->use(transform);
	//Render
	GL_CALL(glDrawElements(faceType, faceSize, GL_UNSIGNED_INT, (void *)(byteOffset)));
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