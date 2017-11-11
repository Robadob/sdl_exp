#include "SceneGraphJoint.h"

SceneGraphJoint::SceneGraphJoint(const glm::mat4 &attachmentTransform)
{
	setModelMat(attachmentTransform);
}
std::shared_ptr<SceneGraphJoint> SceneGraphJoint::make(const glm::mat4 &attachmentTransform)
{
	return std::shared_ptr<SceneGraphJoint>(new SceneGraphJoint(attachmentTransform));
}
//std::shared_ptr<SceneGraphJoint> SceneGraphJoint::clone()
//{
//	return std::shared_ptr<SceneGraphJoint>(new SceneGraphJoint(*this));
//}
///**
//* Copy constructor
//* @TODO
//*/
//SceneGraphJoint::SceneGraphJoint(const SceneGraphJoint& b)
//	: SceneGraphVertex(b)
//{ }
SceneGraphJoint::SceneGraphJoint(SceneGraphJoint&& b)
	: SceneGraphVertex(std::move(b))
{ }
///**
//* Copy assignment operator
//* @TODO?
//*/
//SceneGraphJoint& SceneGraphJoint::operator= (const SceneGraphJoint& b)
//{
//	SceneGraphVertex::operator=(b);
//}
SceneGraphJoint& SceneGraphJoint::operator= (SceneGraphJoint&& b)
{
	SceneGraphVertex::operator=(std::move(b));
	return *this;
}






//Handsy behaviour
//Perhaps make a different type of joint or attach method?
//SceneGraphEdge& SceneGraphEdge::grab(const std::shared_ptr<SceneGraphVertex> &target)
//{
//	assert(target->attachmentCount() == 0);//If this is attached elsewhere when released it will have a bad effect as model mat chagnes
//	return vertex->addChild(target, inverse(computedGlobalTransform));
//}
//bool SceneGraphEdge::release(const std::shared_ptr<SceneGraphVertex> &target)
//{
//	const SceneGraphEdge *edge = vertex->getChildEdge(target);
//	if (edge)
//	{
//		assert(target->attachmentCount() == 1);//Changing model matrix will do bad stuff if this is attached elsewhere
//		target->setModelMat(computedGlobalTransform*edge->localTransform*target->getModelMat());
//		vertex->removeChild(*edge);
//		return true;
//	}
//	return false;
//}