#ifndef __ModelNode_h__
#define __ModelNode_h__
#include <vector>
#include <memory>
#include "GLcheck.h"
#include "Shaders.h"

struct ModelData;
class ModelNode;
/**
* Mesh of a component
*/
class Mesh
{
	friend class ModelNode;
public:
	static std::shared_ptr<Mesh> make_shared(std::shared_ptr<ModelData> data, unsigned int fo, unsigned int fc, unsigned int mi, GLenum ft)
	{
		std::shared_ptr<Mesh> rtn = std::shared_ptr<Mesh>(new Mesh(data, fo, fc, mi, ft));
		rtn->me = rtn;
		return rtn;
	}
	void render(std::shared_ptr<Shaders> shaders);
private:
	std::weak_ptr<Mesh> me;
	std::shared_ptr<Mesh> shared_ptr() const
	{
		return me.lock();
	}
	Mesh(std::shared_ptr<ModelData> data, unsigned int bo, unsigned int fs, unsigned int mi, GLenum ft)
		: byteOffset(bo)
		, faceSize(fs)
		, materialIndex(mi)
		, faceType(ft)
		, data(data)
	{ }
	void setParent(std::shared_ptr<ModelNode> parent)
	{
		this->parent = parent;
	}
private:
	unsigned int byteOffset;
	unsigned int faceSize;
	unsigned int materialIndex;

	GLenum faceType;//GL_POINT, GL_LINE, GL_TRIANGLE, GL_QUAD, GL_POLYGON, GL_TRIANGLE_STRIP?
	std::shared_ptr<ModelData> data;
	std::weak_ptr<ModelNode> parent;
};

/**
 * Hierarchical component of a model
 */
class ModelNode
{
public:
	static std::shared_ptr<ModelNode> make_shared(std::shared_ptr<ModelData> data, unsigned int to)
	{
		std::shared_ptr<ModelNode> rtn = std::shared_ptr<ModelNode>(new ModelNode(data, to));
		rtn->me = rtn;
		return rtn;
	}

private:
	std::weak_ptr<ModelNode> me;
	std::shared_ptr<ModelNode> shared_ptr() const
	{
		return me.lock();
	}
	ModelNode(std::shared_ptr<ModelData> data, unsigned int to)
		: transformOffset(to)
	    , data(data)
	{ }
public:
	void render(std::shared_ptr<Shaders> shaders, glm::mat4 transform);
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
private:
	unsigned int transformOffset;

	std::weak_ptr<ModelNode> parent;
	std::shared_ptr<ModelData> data;
	std::vector<std::shared_ptr<ModelNode>> children;
	std::vector<std::shared_ptr<Mesh>> meshes;
};

#endif