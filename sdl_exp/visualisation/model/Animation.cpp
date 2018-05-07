#include "Animation.h"

glm::vec3 Animation::NodeAnimation::calcInterpolatedScaling(float time) const
{
	//Don't lerp with a single value
	if (this->scalingKeyCount == 1) {
		return this->scalingKeys[0].vec3;
	}

	unsigned int scalingIndex = 0;
	for (; scalingIndex < this->scalingKeyCount; ++scalingIndex)
	{
		if (time < this->scalingKeys[scalingIndex + 1].time) {
			break;
		}
	}
	unsigned int nextScalingIndex = (scalingIndex + 1);
	assert(nextScalingIndex < this->scalingKeyCount);
	float deltaTime = this->scalingKeys[nextScalingIndex].time - this->scalingKeys[scalingIndex].time;
	float factor = (time - this->scalingKeys[scalingIndex].time) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);
	const glm::vec3& startScale = this->scalingKeys[scalingIndex].vec3;
	const glm::vec3& endScale = this->scalingKeys[nextScalingIndex].vec3;
	return glm::mix(startScale, endScale, factor);
}
glm::quat Animation::NodeAnimation::calcInterpolatedRotation(float time) const
{
	//Don't lerp with a single value
	if (this->rotationKeyCount == 1) {
		return this->rotationKeys[0].rotation;
	}

	unsigned int rotationIndex = 0;
	for (; rotationIndex < this->rotationKeyCount - 1; ++rotationIndex)
	{
		if (time < this->rotationKeys[rotationIndex + 1].time) {
			break;
		}
	}
	unsigned int nextRotationIndex = (rotationIndex + 1);
	assert(nextRotationIndex < this->rotationKeyCount);
	float deltaTime = this->rotationKeys[nextRotationIndex].time - this->rotationKeys[rotationIndex].time;
	float factor = (time - this->rotationKeys[rotationIndex].time) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);
	const glm::quat& StartRotationQ = this->rotationKeys[rotationIndex].rotation;
	const glm::quat& EndRotationQ = this->rotationKeys[nextRotationIndex].rotation;
	return glm::normalize(glm::slerp(StartRotationQ, EndRotationQ, factor));
}
glm::vec3 Animation::NodeAnimation::calcInterpolatedTranslation(float time) const
{
	//Don't lerp with a single value
	if (this->positionKeyCount == 1) {
		return this->positionKeys[0].vec3;
	}

	unsigned int translationIndex = 0;
	for (; translationIndex < this->positionKeyCount; ++translationIndex)
	{
		if (time < this->positionKeys[translationIndex + 1].time) {
			break;
		}
	}
	unsigned int nextTranslationIndex = (translationIndex + 1);
	assert(nextTranslationIndex < this->positionKeyCount);
	float deltaTime = this->positionKeys[nextTranslationIndex].time - this->positionKeys[translationIndex].time;
	float factor = (time - this->positionKeys[translationIndex].time) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);
	const glm::vec3& startPos = this->positionKeys[translationIndex].vec3;
	const glm::vec3& endPos = this->positionKeys[nextTranslationIndex].vec3;
	return glm::mix(startPos, endPos, factor);
}
glm::vec3 Animation::NodeAnimation::calcInterpolatedScalingTo(
	const unsigned int &iStart,
	const NodeAnimation *end,
	const unsigned int &iEnd,
	const float &factor) const
{
	assert(iStart < this->scalingKeyCount);
	assert(iEnd < end->scalingKeyCount);
	const glm::vec3& startScale = this->scalingKeys[iStart].vec3;
	const glm::vec3& endScale = end->scalingKeys[iEnd].vec3;
	return glm::mix(startScale, endScale, factor);
}
glm::quat Animation::NodeAnimation::calcInterpolatedRotationTo(
	const unsigned int &iStart,
	const NodeAnimation *end,
	const unsigned int &iEnd,
	const float &factor) const
{
	assert(iStart < this->rotationKeyCount);
	assert(iEnd < end->rotationKeyCount);
	const glm::quat& startRotationQ = this->rotationKeys[iStart].rotation;
	const glm::quat& endRotationQ = end->rotationKeys[iEnd].rotation;
	return glm::normalize(glm::slerp(startRotationQ, endRotationQ, factor));
}
glm::vec3 Animation::NodeAnimation::calcInterpolatedTranslationTo(
	const unsigned int &iStart,
	const NodeAnimation *end,
	const unsigned int &iEnd,
	const float &factor) const
{
	assert(iStart < this->positionKeyCount);
	assert(iEnd < end->positionKeyCount);
	const glm::vec3& startPos = this->positionKeys[iStart].vec3;
	const glm::vec3& endPos = end->positionKeys[iEnd].vec3;
	return glm::mix(startPos, endPos, factor);
}
/**
 * Constructors & Destructors
 */
Animation::~Animation()
{
	for (auto &a : nodeAnims)
		delete a.second;
	nodeAnims.clear();
	for (auto &b : meshAnims)
		delete b.second;
	meshAnims.clear();
	for (auto &b : meshMorphAnims)
		delete b.second;
	meshMorphAnims.clear();
}
Animation::NodeAnimation::NodeAnimation(unsigned int p, unsigned int r, unsigned int s)
	: positionKeyCount(p)
	, positionKeys((Vec3Key*)malloc(p * sizeof(Vec3Key)))
	, rotationKeyCount(r)
	, rotationKeys((RotationKey*)malloc(r * sizeof(RotationKey)))
	, scalingKeyCount(s)
	, scalingKeys((Vec3Key*)malloc(s * sizeof(Vec3Key)))
{
	memset(positionKeys, 0, p * sizeof(Vec3Key));
	memset(rotationKeys, 0, r * sizeof(RotationKey));
	for (unsigned int i = 0; i < r; ++i)
		rotationKeys[i].rotation.w = 1.0f;
	for (unsigned int i = 0; i < s; ++i)
		scalingKeys[i].vec3 = glm::vec3(1.0f);
}
Animation::NodeAnimation::~NodeAnimation()
{
	free(positionKeys);
	free(rotationKeys);
	free(scalingKeys);
}
Animation::MeshAnimation::MeshAnimation(unsigned int count)
	: keyCount(count)
	, meshKeys((Key*)malloc(count * sizeof(Key)))
{
}
Animation::MeshAnimation::~MeshAnimation()
{
	free(meshKeys);
}
Animation::MeshMorphAnimation::Key::Key(unsigned int count, float time)
	: time(time)
	, count(count)
	, values((unsigned int*)malloc(count * sizeof(unsigned int)))
	, weights((float*)malloc(count * sizeof(float)))
{

}
Animation::MeshMorphAnimation::Key::~Key()
{
	free(values);
	free(weights);
}
Animation::MeshMorphAnimation::MeshMorphAnimation(unsigned int count)
	: keyCount(count)
	, meshKeys((Key*)malloc(count * sizeof(Key)))
{
}
Animation::MeshMorphAnimation::~MeshMorphAnimation()
{
	for (unsigned int i = 0; i < keyCount; ++i)
		meshKeys[i].~Key();//Must be manually destructed
	free(meshKeys);
}