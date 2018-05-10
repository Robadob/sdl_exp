#ifndef __Mesh_h__
#define __Mesh_h__
#include <memory>
#include "../shader/Shaders.h"
#include "BoundingBox.h"
#include <unordered_map>
struct ModelData;
class ModelNode;

/**
* Renderable Mesh of a component
*/
class Mesh
{
	friend class ModelNode;
public:
	static std::shared_ptr<Mesh> make_shared(const char *name, std::shared_ptr<ModelData> data, unsigned int fo, unsigned int fc, unsigned int mi, GLenum ft, unsigned int boneOffset = 0, unsigned int boneCount = 0)
	{
		std::shared_ptr<Mesh> rtn = std::shared_ptr<Mesh>(new Mesh(name, data, fo, fc, mi, ft, boneOffset, boneCount));
		rtn->me = rtn;
		return rtn;
	}
	void render(glm::mat4 &transform, const std::shared_ptr<Shaders> &shader = nullptr) const;
	BoundingBox3D calculateBoundingBox(glm::mat4 transform) const;
	std::string getName() const { return name; }
	void setVisible(bool isVisible) { this->visible = isVisible; }
	bool getVisible() const { return this->visible; }
private:
	std::weak_ptr<Mesh> me;
	std::shared_ptr<Mesh> shared_ptr() const
	{
		return me.lock();
	}
	Mesh(const char *name, std::shared_ptr<ModelData> data, unsigned int bo, unsigned int fs, unsigned int mi, GLenum ft, unsigned int boneOffset = 0, unsigned int boneCount = 0)
		: name(name)
		, byteOffset(bo)
		, faceSize(fs)
		, materialIndex(mi)
		, visible(true)
		, faceType(ft)
		, data(data)
	{ }
	void setParent(std::shared_ptr<ModelNode> parent)
	{
		this->parent = parent;
	}
	std::string name;
	unsigned int byteOffset;
	unsigned int faceSize;
	unsigned int materialIndex;
	bool visible;

	GLenum faceType;//GL_POINT, GL_LINE, GL_TRIANGLE, GL_QUAD, GL_POLYGON, GL_TRIANGLE_STRIP?
	std::shared_ptr<ModelData> data;
	std::weak_ptr<ModelNode> parent;
};
#endif //__Mesh_h__