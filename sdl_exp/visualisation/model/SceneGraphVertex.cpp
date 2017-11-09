#include "SceneGraphVertex.h"
#include "SceneGraphEdge.h"

SceneGraphVertex::SceneGraphVertex()
	:attachments(0)
{

}
/**
* Unsure if this will work, can we delete a reference?
* Can we get the pointer to an iterator and compare with pointer to reference?
*/
void SceneGraphVertex::removeChild(const SceneGraphEdge &edge)
{
	for (auto &&it = children.begin(); it != children.end(); ++it)
	{
		if (&(*it) == &edge)
		{
			children.erase(it);
			return;
		}
	}
}
void SceneGraphVertex::propagateUpdate(const glm::mat4 &parentTransform)
{
	for (SceneGraphEdge &edge : children)
	{
		edge.propagateUpdate(parentTransform);
	}
}
const SceneGraphEdge *SceneGraphVertex::getChildEdge(const std::shared_ptr<SceneGraphVertex> &child)
{
	for (auto &&it = children.begin(); it != children.end(); ++it)
	{
		if (it->vertex == child)
		{
			return &(*it);
		}
	}
	return nullptr;
}