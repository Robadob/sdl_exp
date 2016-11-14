#include "ModelNode.h"
#include "Model.h"
void Mesh::render(std::shared_ptr<Shaders> shaders)
{
	//Apply material

	//Render
	GL_CALL(glDrawElements(faceType, faceSize, GL_UNSIGNED_INT, (void *)faceOffset));
	//char *a="";
	//if (faceType == GL_POINTS)
	//	a = "points";
	//if (faceType == GL_LINES)
	//	a = "lines";
	//if (faceType == GL_TRIANGLES)
	//	a = "triangles";

	//printf("(%s, %d, %d)\n", a, faceSize, faceOffset);
	//GL_CALL(glDrawRangeElements(faceType, (void *)faceOffset, (void *)faceOffset + faceCount, faceCount, GL_UNSIGNED_INT, 0));
}
void ModelNode::render(std::shared_ptr<Shaders> shaders, glm::mat4 transform)
{
	//Calculate & apply transform
	transform = data->transforms[transformOffset] * transform;
	//transform = transform * data->transforms[transformOffset];
	shaders->setModelMat(transform);

	//Render all meshes
	for (auto &&mesh:meshes)
	{
		mesh->render(shaders);
	}

	//Recursively render all children
	for (auto &&child : children)
	{
		child->render(shaders, transform);
	}
}
