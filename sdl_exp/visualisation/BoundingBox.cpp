
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "BoundingBox.h"


BoundingBox::BoundingBox(const aiScene* scene)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	aiVector3D min, max;
	min.x = min.y = min.z = 1e10f;
	max.x = max.y = max.z = -1e10f;
	calcBoundingBoxForNode(scene, scene->mRootNode, &min, &max, &trafo);

	this->min = glm::vec3(min.x, min.y, min.z);
	this->max = glm::vec3(max.x, max.y, max.z);
	this->max = glm::vec3(max.x, max.y, max.z);
	this->center = (this->min + this->max) / 2.0f;
}

void BoundingBox::calcBoundingBoxForNode(const aiScene* scene, const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo)
{
	aiMatrix4x4 prev = *trafo;
	unsigned int n = 0, t;

	aiMultiplyMatrix4(trafo, &nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, trafo);

			min->x = glm::min(min->x, tmp.x);
			min->y = glm::min(min->y, tmp.y);
			min->z = glm::min(min->z, tmp.z);

			max->x = glm::max(max->x, tmp.x);
			max->y = glm::max(max->y, tmp.y);
			max->z = glm::max(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		calcBoundingBoxForNode(scene, nd->mChildren[n], min, max, trafo);
	}
	*trafo = prev;
}