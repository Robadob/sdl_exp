#include "SceneGraph.h"

SceneGraphEdge::SceneGraphEdge(const std::shared_ptr<SceneGraphVertex> &vertex, const glm::mat4 &localTransform)
    : vertex(vertex)
    , localTransform(localTransform)
    , computedGlobalTransform(1)
    , hasChanged(true)//Set changed as we lack parent transforms
{  }
    
inline void SceneGraphEdge::propagateUpdate(const glm::mat4 &parentTransform)
{
    //Update my transform
    computedGlobalTransform = parentTransform * localTransform;
    //Reset flag
    hasChanged = false;
    //Propagate to children
    vertex->propagateUpdate(computedGlobalTransform);
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
void SceneGraphVertex::propagateUpdate(const glm::mat4 &parentTransform)
{
    for (SceneGraphEdge &edge : children)
    {
        edge.propagateUpdate(parentTransform);
    }
}