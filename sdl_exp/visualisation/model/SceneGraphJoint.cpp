#include "SceneGraphJoint.h"

SceneGraphJoint::SceneGraphJoint(const glm::mat4 &attachmentTransform)
{
    setSceneMat(attachmentTransform);
}
std::shared_ptr<SceneGraphJoint> SceneGraphJoint::make(const glm::mat4 &attachmentTransform)
{
    return std::shared_ptr<SceneGraphJoint>(new SceneGraphJoint(attachmentTransform));
}

//std::shared_ptr<SceneGraphJoint> SceneGraphJoint::clone()
//{
//    return std::shared_ptr<SceneGraphJoint>(new SceneGraphJoint(*this));
//}
///**
//* Copy constructor
//* @TODO
//*/
//SceneGraphJoint::SceneGraphJoint(const SceneGraphJoint& b)
//    : SceneGraphItem(b)
//{ }
SceneGraphJoint::SceneGraphJoint(SceneGraphJoint&& b)
    : SceneGraphItem(std::move(b))
{ }
///**
//* Copy assignment operator
//* @TODO?
//*/
//SceneGraphJoint& SceneGraphJoint::operator= (const SceneGraphJoint& b)
//{
//    SceneGraphItem::operator=(b);
//}
SceneGraphJoint& SceneGraphJoint::operator= (SceneGraphJoint&& b)
{
    SceneGraphItem::operator=(std::move(b));
    return *this;
}


/////////////////////////////////////////////////////////////////////////
//                 ~Scene Graph Attachment Management~                 //
// Joints have no attachment points, so we treat it slightly different //
/////////////////////////////////////////////////////////////////////////
bool SceneGraphJoint::attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, unsigned int childAttachOffsetIndex)
{
    if (child)
        return SceneGraphItem::attach(child, reference, glm::vec3(0), child->getAttachmentOffset(childAttachOffsetIndex));
    return false;//Shared ptr is empty
}
bool SceneGraphJoint::attach(const std::shared_ptr<SceneGraphItem> &child, const std::string &reference, glm::vec3 childAttachOffset)
{
    return SceneGraphItem::attach(child, reference, glm::vec3(0), childAttachOffset);
}
//SceneGraphItem special versions for SceneGraphJoint
bool SceneGraphItem::attach(const std::shared_ptr<SceneGraphJoint> &child, const std::string &reference, glm::vec3 parentAttachOffset)
{
    return attach(child, reference, parentAttachOffset, glm::vec3(0));
}
bool SceneGraphItem::attach(const std::shared_ptr<SceneGraphJoint> &child, const std::string &reference, unsigned int parentAttachOffsetIndex)
{
    return attach(child, reference, this->getAttachmentOffset(parentAttachOffsetIndex), glm::vec3(0));
}



//Handsy behaviour
//Perhaps make a different type of joint or attach method?
//SceneGraphEdge& SceneGraphEdge::grab(const std::shared_ptr<SceneGraphItem> &target)
//{
//    assert(target->attachmentCount() == 0);//If this is attached elsewhere when released it will have a bad effect as model mat chagnes
//    return vertex->addChild(target, inverse(computedGlobalTransform));
//}
//bool SceneGraphEdge::release(const std::shared_ptr<SceneGraphItem> &target)
//{
//    const SceneGraphEdge *edge = vertex->getChildEdge(target);
//    if (edge)
//    {
//        assert(target->attachmentCount() == 1);//Changing model matrix will do bad stuff if this is attached elsewhere
//        target->setSceneMat(computedGlobalTransform*edge->localTransform*target->getSceneMat());
//        vertex->removeChild(*edge);
//        return true;
//    }
//    return false;
//}