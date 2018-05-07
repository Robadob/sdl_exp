#ifndef __ModelNode_h__
#define __ModelNode_h__
#include <vector>
#include <memory>
#include "../util/GLcheck.h"
#include "../shader/Shaders.h"
#include "Mesh.h"
#include "BoundingBox.h"
#include "Animation.h"

struct ModelData;

/**
* Hierarchical component of a model
*/
class ModelNode
{
public:
	static std::shared_ptr<ModelNode> make_shared(std::shared_ptr<ModelData> data, unsigned int to, const char * name)
	{
		std::shared_ptr<ModelNode> rtn = std::shared_ptr<ModelNode>(new ModelNode(data, to, name));
		rtn->me = rtn;
		return rtn;
	}

private:
	std::weak_ptr<ModelNode> me;
	std::shared_ptr<ModelNode> shared_ptr() const
	{
		return me.lock();
	}
	ModelNode(std::shared_ptr<ModelData> data, unsigned int to, const char *name)
		: transformOffset(to)
		, name(name)
		, data(data)
	{ }
public:
	BoundingBox3D calculateBoundingBox(glm::mat4 transform = glm::mat4());
	void render(glm::mat4 transform);
	//debugging test method
	void renderSkeleton(glm::mat4 transform, glm::vec4 pt0 = glm::vec4(0, 0, 0, 1));
	void addChild(std::shared_ptr<ModelNode> child)
	{
		this->children.push_back(child);
		child->setParent(me.lock());
	}
	void addMesh(std::shared_ptr<Mesh> mesh)
	{
		this->meshes.push_back(mesh);
		mesh->setParent(me.lock());
	}
	void setParent(std::shared_ptr<ModelNode> parent)
	{
		this->parent = parent;
	}
	std::string getName() const { return name; }

	const std::vector<std::shared_ptr<ModelNode>> *getChildren() const { return &children; };
	const std::vector<std::shared_ptr<Mesh>> *getMeshes() const { return &meshes; };
	void propagateAnimation(const float time, const unsigned int animId = 0, glm::mat4 parentTransform = glm::mat4(1));
	void propagateKeyframeInterpolation(const Animation::NodeKeyMap &start, const unsigned int &startFrameIndex, const Animation::NodeKeyMap &end, const unsigned int &endFrameIndex, const float &factor, glm::mat4 parentTransform = glm::mat4(1));
	void constructRootChain(std::vector<unsigned int> &hierarchy);
	unsigned int transformOffset;
private:
	//name is how we match bones to the hierarchy
	std::string name;
	std::weak_ptr<ModelNode> parent;
	std::shared_ptr<ModelData> data;
public:
	std::vector<std::shared_ptr<ModelNode>> children;
	std::vector<std::shared_ptr<Mesh>> meshes;
};

#endif