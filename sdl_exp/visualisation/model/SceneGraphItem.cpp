#include "SceneGraphItem.h"

SceneGraphItem::SceneGraphItem()
	:expired(false)
{ }
//SceneGraphItem::SceneGraphItem(const SceneGraphItem& b)
//	: parents(b.parents)
//	, children(children)
//	, expired(b.expired)
//	, modelMat(b.modelMat)
//{
//	//TODO: Decide how to handle children/parents on copy
//	//If we copy construct a SGV we need to handle what will happen to parents/children
//	//Drop parents, clone children?
//}
/**
* Move constructor
* @TODO?
*/
SceneGraphItem::SceneGraphItem(SceneGraphItem&& b)
	: parents(b.parents)
	, children(children)
	, expired(b.expired)
	, modelMat(b.modelMat)
{
	//Steal children from b
	for (auto &it : children)
	{
		it.child->removeParent(b.shared_from_this());
		it.child->addParent(this->shared_from_this());
	}
	b.children.clear();
	//Tell parents to switch child from b to me
	for (auto &it : parents)
	{
		if (auto a = it.lock())
		{
			for (auto &itC : a->children)
			{
				if (itC.child.get() == &b)
				{
					//Switch child to me
					itC.child = this->shared_from_this();
				}
			}
		}	
	}
	b.parents.clear();
}
//SceneGraphItem& SceneGraphItem::operator = (const SceneGraphItem& b)
//{
//	//TODO: Decide how to handle children/parents on copy
//	//If we copy construct a SGV we need to handle what will happen to parents/children
//	//Drop parents, clone children?
//  return *this;
//}
/**
* Move assignment operator
* @TODO?
*/
SceneGraphItem& SceneGraphItem::operator= (SceneGraphItem&& b)
{
	this->parents = b.parents;
	this->children = children;
	this->expired = b.expired;
	this->modelMat = b.modelMat;
	//Steal children from b
	for (auto &it : children)
	{
		it.child->removeParent(b.shared_from_this());
		it.child->addParent(this->shared_from_this());
	}
	b.children.clear();
	//Tell parents to switch child from b to me
	for (auto &it : parents)
	{
		if (auto a = it.lock())
		{
			for (auto &itC : a->children)
			{
				if (itC.child.get() == &b)
				{
					itC.child = this->shared_from_this();
				}
			}
		}
	}
	b.parents.clear();
	return *this;
}
void SceneGraphItem::propagateUpdate(const glm::mat4 &sceneTransform)
{
	//Recompute child transforms recursively
	if (children.size())
	{
		glm::mat4 sceneModelTransform = sceneTransform * modelMat;
		for (auto &it : children)
		{
			it.setComputedTransformMat(sceneModelTransform);
			it.child->propagateUpdate(it.computedTransformMat);
		}
	}
	//Nolonger expired!
	expired = false;
}
void SceneGraphItem::renderSceneGraph(const glm::mat4 &rootTransform, const glm::mat4 &computedSceneTransform)
{
	//If expired, compute each child's tranform mat from scene transform (and model mat)
	if (expired)
		propagateUpdate(computedSceneTransform);
	//Render this
	render(rootTransform * computedSceneTransform);
	//Render Children
	for (auto &it : children)
	{
		//Recursively render childs scene graph
		it.child->renderSceneGraph(rootTransform, it.computedTransformMat);
	}
}
///////////////////////////////////////
// Scene Graph Attachment Management //
///////////////////////////////////////
bool SceneGraphItem::attach(
	const std::shared_ptr<SceneGraphItem> &child, 
	const std::string &reference, 
	unsigned int parentAttachOffsetIndex, 
	unsigned int childAttachOffsetIndex
) {
	return attach(
		child,
		reference,
		this->getAttachmentOffset(parentAttachOffsetIndex),
		child->getAttachmentOffset(childAttachOffsetIndex)
		);
}
bool SceneGraphItem::attach(
	const std::shared_ptr<SceneGraphItem> &child,
	const std::string &reference, 
	glm::vec3 parentAttachOffset, 
	glm::vec3 childAttachOffset
) {
//Recursive checks are expensive, we trust release mode to do things properly
#ifdef _DEBUG
	//Recurse scene graph to ensure we won't be creating a cycle
	{
		std::list<std::shared_ptr<SceneGraphItem>> prospectiveChildren = child->getChildPtrs();
		for (const auto &itC : prospectiveChildren)
		{
			//Check if child is me
			if (itC.get() == this)
				return false;
			//Check if child is parent
			if (hasParentAttachment(itC))
				return false;
		}
	}
#endif
	//Add child as new attachment
	{
		//Add me as parent
		child->addParent(this->shared_from_this());
		//Add to me as child
		children.insert(children.begin(), { reference, child, parentAttachOffset, childAttachOffset, glm::mat4(1) });
		//Mark this node as expired so it gets recursively updated at next render
		expired = true;
	}

	return true;
}
bool SceneGraphItem::detach(const std::string &reference)
{
	//Check my children
	for (auto &&it = children.begin(); it != children.end(); ++it)
	{
		if (it->reference == reference)
		{
			//Remove me as parent
			it->child->removeParent(this->shared_from_this());
			//Remove child from me
			children.erase(it);
			return true;
		}
	}
	//Else recurse
	//Depth first isn't ideal, but ideally we won't have duplicate references
	//Too much effort however to prevent duplicate references, e.g. joining and copying branches of scene graphs
	for (const auto &it:children)
	{
		if (it.child->detach(reference))
			return true;
	}
	//Else false
	return false;
}
std::shared_ptr<SceneGraphItem> SceneGraphItem::getAttachment(const std::string &reference)
{
	//Check my children
	for (auto &&it = children.begin(); it != children.end(); ++it)
	{
		if (it->reference == reference)
		{
			return it->child;
		}
	}
	//Else recurse
	//Depth first isn't ideal, but ideally we won't have duplicate references
	//Too much effort however to prevent duplicate references, e.g. joining and copying branches of scene graphs
	for (const auto &it : children)
	{
		if (auto a = it.child->getAttachment(reference))
			return a;
	}
	//Else failure
	return nullptr;
}
bool SceneGraphItem::hasChildAttachment(const std::shared_ptr<const SceneGraphItem> &child) const
{
	//Check my children
	for (auto &&it = children.begin(); it != children.end(); ++it)
	{
		if (it->child == child)
		{
			return true;
		}
	}
	//Else recurse
	//Depth first isn't ideal, but ideally we won't have duplicate references
	//Too much effort however to prevent duplicate references, e.g. joining and copying branches of scene graphs
	for (const auto &it : children)
	{
		if (it.child->hasChildAttachment(child))
			return true;
	}
	//Else failure
	return false;
}
/////////////////////////////////
// Scene Graph Parent Tracking //
// --prevents cycle creation-- //
/////////////////////////////////
bool SceneGraphItem::hasParentAttachment(const std::shared_ptr<const SceneGraphItem> &parent) const
{
	//Check my parents
	for (auto &&it = parents.begin(); it != parents.end(); ++it)
	{
		if (it->lock() == parent)
		{
			return true;
		}
	}
	//Else recurse
	for (auto &&it = parents.begin(); it != parents.end(); ++it)
	{
		if (auto a = it->lock())
		{
			if (a->hasParentAttachment(parent))
				return true;
		}
	}
	//Else failure
	return false;
}
void SceneGraphItem::addParent(const std::shared_ptr<SceneGraphItem> &parent)
{
	parents.push_back(parent);
}
void SceneGraphItem::removeParent(const std::shared_ptr<const SceneGraphItem> &parent)
{
	for (auto &&it = parents.begin(); it != parents.end(); ++it)
	{
		if (it->lock() == parent)
		{
			//Remove ONE parent from me
			parents.erase(it);
			return;
		}
	}
	//If we don't have a parent to remove, that's a problem!
	assert(false);
}
#ifdef _DEBUG
std::list<std::shared_ptr<SceneGraphItem>> SceneGraphItem::getChildPtrs() const
{
	std::list<std::shared_ptr<SceneGraphItem>> rtn;
	for (const auto &it : children)
	{
		//Add child
		rtn.push_back(it.child);
		//Recurse child's children
		const std::list<std::shared_ptr<SceneGraphItem>> &a = it.child->getChildPtrs();
		rtn.insert(rtn.end(), a.begin(), a.end());
	}
	return rtn;
}
#endif