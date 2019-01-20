#include "ModelNode.h"
#include "Model.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../util/StringUtils.h"
#include <glm/gtx/matrix_decompose.hpp>

void ModelNode::render(glm::mat4 transform, const unsigned int &shaderIndex)
{
	//Calculate & apply transform
	//using data->_transforms here doesn't make a difference? but required to animate renderSkeleton() 
	transform *= data->transforms[transformOffset];

	//Render all meshes
	for (auto &&mesh : meshes)
	{
		mesh->render(transform, shaderIndex);
	}
	//Recursively render all children
	for (auto &&child : children)
	{
		child->render(transform, shaderIndex);
	}
}
void ModelNode::renderSkeleton(Draw &pen, glm::mat4 parentTransform, glm::vec4 pt0)
{
	//Calculate & apply transform
	parentTransform *= data->_transforms[transformOffset];

	glm::vec4 pt1 = pt0;
	if (data->boneMapping.find(name)!=data->boneMapping.end())
	{
		//Transform point into node space
		pt1 = parentTransform * glm::vec4(0, 0, 0, 1);
		if (pt0!=glm::vec4(0,0,0,1))
		{//If parentPoint has been set
			//Render bone limb
			pen.vertex3(&pt0);
			pen.vertex3(&pt1);
		}
	}

	for (auto &c : children) {
		c->renderSkeleton(pen, parentTransform, pt1);
	}
}
BoundingBox3D ModelNode::calculateBoundingBox(glm::mat4 transform)
{
    BoundingBox3D rtn;
    //Calculate & apply transform
    transform *= data->transforms[transformOffset];

    //Include all meshes
    for (auto &&mesh : meshes)
    {
        rtn.include(mesh->calculateBoundingBox(transform));
    }

    //Recursively include all children
    for (auto &&child : children)
    {
        rtn.include(child->calculateBoundingBox(transform));
    }
    return rtn;
}
void ModelNode::propagateAnimation(const float time, const unsigned int animId, glm::mat4 parentTransform)
{
	const Animation &anim = *data->animations[animId];

	auto nodeIt = anim.nodeAnims.find(this->name);
	if (nodeIt != anim.nodeAnims.end())
	{//Animation, so calculate inverse skinning transformation
		const Animation::NodeAnimation *nodeAnim = nodeIt->second;
		glm::mat4 S = glm::scale(glm::mat4(1), nodeAnim->calcInterpolatedScaling(time));
		glm::mat4 R = glm::toMat4(nodeAnim->calcInterpolatedRotation(time));
		glm::mat4 T = glm::translate(glm::mat4(1), nodeAnim->calcInterpolatedTranslation(time));
        
        //Apply tranformations in usual order: Scale, Rotate, Translate
		data->_transforms[transformOffset] = T * R * S;
	}
    else
    {//No animation, so load inverse bind pose transformation
		data->_transforms[transformOffset] = data->transforms[transformOffset];
	}
	parentTransform *= data->_transforms[transformOffset];

	auto boneIt = data->boneMapping.equal_range(name);
	for (auto i = boneIt.first; i != boneIt.second; ++i) {
		//FBX models don't set data->inverseRootTransform automatically, so it might be calculated wrong
		data->computedTransforms[i->second] = data->inverseRootTransform * parentTransform * data->boneMatrices[i->second];
	}

	for (auto &c : children) {
        c->propagateAnimation(time, animId, parentTransform);
	}
}

void ModelNode::propagateKeyframeInterpolation(
	const Animation::NodeKeyMap &start, 
	const unsigned int &iStart, 
	const Animation::NodeKeyMap &end, 
	const unsigned int &iEnd, 
	const float &factor,
	glm::mat4 parentTransform
	)
{
	auto sNodeIt = start.find(this->name);
	auto eNodeIt = end.find(this->name);
	if (sNodeIt != start.end() && eNodeIt != end.end())
	{//Animation, so calculate inverse skinning transformation
		const Animation::NodeAnimation *sNodeAnim = sNodeIt->second;
		const Animation::NodeAnimation *eNodeAnim = eNodeIt->second;
		glm::mat4 S = glm::scale(glm::mat4(1), sNodeAnim->calcInterpolatedScalingTo(iStart, eNodeAnim, iEnd, factor));
		glm::mat4 R = glm::toMat4(sNodeAnim->calcInterpolatedRotationTo(iStart, eNodeAnim, iEnd, factor));
		glm::mat4 T = glm::translate(glm::mat4(1), sNodeAnim->calcInterpolatedTranslationTo(iStart, eNodeAnim, iEnd, factor));

		//Apply tranformations in usual order: Scale, Rotate, Translate
		data->_transforms[transformOffset] = T * R * S;
	}
	else if (sNodeIt != start.end())//unsure why this seems inverted
	{//Animation with no end pose, so calculate inverse skinning transformation to bind pose
		glm::vec3 scale, translation, skew;
		glm::quat orientation;
		glm::vec4 perspective;
		decompose(data->transforms[transformOffset], scale, orientation, translation, skew, perspective);
		orientation = -orientation;
		orientation.w = -orientation.w;//Unsure why we need to negate x,y,z of quaternion for correct rotation, but it works
		const Animation::NodeAnimation *sNodeAnim = sNodeIt->second;
		assert(iStart < sNodeAnim->scalingKeyCount);
		assert(iStart < sNodeAnim->rotationKeyCount);
		assert(iStart < sNodeAnim->positionKeyCount);
		glm::mat4 S = glm::scale(glm::mat4(1), glm::mix(sNodeAnim->scalingKeys[iEnd].vec3, scale, factor));
		glm::mat4 R = glm::toMat4(glm::normalize(glm::slerp(sNodeAnim->rotationKeys[iEnd].rotation, orientation, factor)));
		glm::mat4 T = glm::translate(glm::mat4(1), glm::mix(sNodeAnim->positionKeys[iEnd].vec3, translation, factor));
		//Apply tranformations in usual order: Scale, Rotate, Translate
		data->_transforms[transformOffset] = T * R * S;
	}
	else if (eNodeIt != end.end())//We create all poses when making the transition start keyframe
	{//Animation with no start pose, so calculate inverse skinning transformation from bind pose
		glm::vec3 scale, translation, skew;
		glm::quat orientation;
		glm::vec4 perspective;
		decompose(data->transforms[transformOffset], scale, orientation, translation, skew, perspective);
		orientation = -orientation;
		orientation.w = -orientation.w;//Unsure why we need to negate x,y,z of quaternion for correct rotation, but it works
		const Animation::NodeAnimation *eNodeAnim = eNodeIt->second;
		assert(iEnd < eNodeAnim->scalingKeyCount);
		assert(iEnd < eNodeAnim->rotationKeyCount);
		assert(iEnd < eNodeAnim->positionKeyCount);
		glm::mat4 S = glm::scale(glm::mat4(1), glm::mix(scale, eNodeAnim->scalingKeys[iEnd].vec3, factor));
		glm::mat4 R = glm::toMat4(glm::normalize(glm::slerp(orientation, eNodeAnim->rotationKeys[iEnd].rotation, factor)));
		glm::mat4 T = glm::translate(glm::mat4(1), glm::mix(translation, eNodeAnim->positionKeys[iEnd].vec3, factor));
		//Apply tranformations in usual order: Scale, Rotate, Translate
		data->_transforms[transformOffset] = T * R * S;
	}
	else
	{//No animation, so load inverse bind pose transformation
		data->_transforms[transformOffset] = data->transforms[transformOffset];
	}
	parentTransform *= data->_transforms[transformOffset];

	auto boneIt = data->boneMapping.equal_range(name);
	for (auto i = boneIt.first; i != boneIt.second; ++i) {
		//FBX models don't set data->inverseRootTransform automatically, so it might be calculated wrong
		data->computedTransforms[i->second] = data->inverseRootTransform * parentTransform * data->boneMatrices[i->second];
	}

	for (auto &c : children) {
		c->propagateKeyframeInterpolation(start, iStart, end, iEnd, factor, parentTransform);
	}
}
void ModelNode::constructRootChain(std::vector<unsigned int> &hierarchy)
{
	hierarchy.push_back(transformOffset);
	unsigned count = 0;
	for (auto &c:children)
	{
		if (su::contains(c->getName(), "root", false))
		{
			if (count<1)
			{
				count++;
				c->constructRootChain(hierarchy);
			}
			else
			{
				fprintf(stderr, "Warning ModelNode::constructRootChain(): Root hierarchy contains multiple potential root chains! Only the first has been traversed.\n");
			}
		}
	}
}