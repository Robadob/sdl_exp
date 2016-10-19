#ifndef __BoundingBox_h__
#define __BoundingBox_h__
#include <glm/glm.hpp>

//Forward declare all ai types
struct aiNode;
struct aiScene;
template <typename T>
class aiVector3t;
typedef aiVector3t<float> aiVector3D;
template <typename T>
class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

class BoundingBox
{
	glm::vec3 min, max, center;
	void calcBoundingBoxForNode(const aiScene* scene, const aiNode* nd,	aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
public:
	BoundingBox(const aiScene *);
	glm::vec3 getMin() { return min; };
	glm::vec3 getMax() { return max; };
	glm::vec3 getCenter() { return center; };
};

#endif //__BoundingBox_h__
