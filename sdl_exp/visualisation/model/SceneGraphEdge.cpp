#include "SceneGraphEdge.h"

SceneGraphEdge::SceneGraphEdge(const std::shared_ptr<SceneGraphVertex> &vertex, const glm::mat4 &attachmentTransform)
    : vertex(vertex)
	, localTransform(attachmentTransform)
    , computedGlobalTransform(1)
    , hasChanged(true)//Set changed as we lack parent transforms
{
	vertex->incrementAttachments();
}
SceneGraphEdge::~SceneGraphEdge()
{
	vertex->decrementAttachments();
}
inline void SceneGraphEdge::propagateUpdate(const glm::mat4 &parentTransform)
{
    //Update my transform
    computedGlobalTransform = parentTransform * localTransform;
    //Reset flag
    hasChanged = false;
    //Propagate to children
    vertex->propagateUpdate(computedGlobalTransform);
}

SceneGraphEdge& SceneGraphEdge::grab(const std::shared_ptr<SceneGraphVertex> &target)
{
	assert(target->attachmentCount() == 0);//If this is attached elsewhere when released it will have a bad effect as model mat chagnes
	return vertex->addChild(target, inverse(computedGlobalTransform));
}
bool SceneGraphEdge::release(const std::shared_ptr<SceneGraphVertex> &target)
{
	const SceneGraphEdge *edge = vertex->getChildEdge(target);
	if (edge)
	{
		assert(target->attachmentCount() == 1);//Changing model matrix will do bad stuff if this is attached elsewhere
		target->setModelMat(computedGlobalTransform*edge->localTransform*target->getModelMat());
		vertex->removeChild(*edge);
		return true;
	}
	return false;
}
bool SceneGraphEdge::release(SceneGraphEdge& target)
{
	return release(target.vertex);
}
void SceneGraphVertex::renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &sceneTransform)
{
    //Render this
    render(rootTransform * sceneTransform);
    //Render Children
    for (SceneGraphEdge &edge : children)
    {
        //Update hierarchy if necessary
        if (edge.expired())
            edge.propagateUpdate(sceneTransform);
        //Recursively render childs scene graph
        edge.renderSceneGraph(rootTransform);
    }
}
SceneGraphEdge &SceneGraphVertex::addChild(const std::shared_ptr<SceneGraphVertex> &child, const glm::mat4 &attachmentTransform = glm::mat4(1))
{
	children.emplace_back(SceneGraphEdge(child, attachmentTransform));
	return *children.end();//C++17 allows to return emplace_back
}