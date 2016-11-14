#include "ModelNode.h"
#include "Model.h"
#include <glm/gtc/type_ptr.hpp>

void Mesh::render(std::shared_ptr<Shaders> shaders)
{
	//int i = 0;
	//i++;
	//if (i<=2)
	//{

		shaders->useProgram();
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < faceSize; ++i)
		{
		/*	if (i % 3 == 0)
				printf("\n");
			printf("(%.4f,%.4f,%.4f)"
				, data->vertices[data->faces[faceOffset + i]].x
				, data->vertices[data->faces[faceOffset + i]].y
				, data->vertices[data->faces[faceOffset + i]].z);*/
			glVertex3fv(glm::value_ptr(data->vertices[data->faces[faceOffset + i]]));
		}
		glEnd();
		GL_CHECK();
	//}
	//Apply material

	//Render
	//GL_CALL(glDrawElements(GL_TRIANGLES, faceSize, GL_UNSIGNED_INT, (void *)faceOffset));
	//GL_CALL(glDrawRangeElements(faceType, faceOffset, faceOffset + faceSize, faceSize, GL_UNSIGNED_INT, (void *)faceOffset));
	//char *a="";
	//if (faceType == GL_POINTS)
	//	a = "points";
	//if (faceType == GL_LINES)
	//	a = "lines";
	//if (faceType == GL_TRIANGLES)
	//	a = "triangles";

	//printf("(%s, %d, %d)\n", a, faceSize, faceOffset);
}
void ModelNode::render(std::shared_ptr<Shaders> shaders, glm::mat4 transform)
{
	//Calculate & apply transform
	transform = glm::mat4();// data->transforms[transformOffset] * transform;
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
