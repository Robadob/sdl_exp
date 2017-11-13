#include "SlideJoint.h"


SlideJoint::SlideJoint(const glm::vec3 &axis, const float &min, const float &max, const float &init)
	: SceneGraphJoint()
	, pos(glm::clamp(init, min, max))
	, min(min)
	, max(max)
	, axis(normalize(axis))
{
	assert(max >= min);
	setOffset(axis*this->pos);
}
std::shared_ptr<SlideJoint> SlideJoint::makeX(const float &min, const float &max, const float &init)
{
	return std::shared_ptr<SlideJoint>(new SlideJoint(glm::vec3(1, 0, 0), min, max, init));
}
std::shared_ptr<SlideJoint> SlideJoint::makeY(const float &min, const float &max, const float &init)
{
	return std::shared_ptr<SlideJoint>(new SlideJoint(glm::vec3(0, 1, 0), min, max, init));
}
std::shared_ptr<SlideJoint> SlideJoint::makeZ(const float &min, const float &max, const float &init)
{
	return std::shared_ptr<SlideJoint>(new SlideJoint(glm::vec3(0, 0, 1), min, max, init));
}
void SlideJoint::set(const float &pos)
{
	this->pos = glm::clamp(pos, min, max);
	setOffset(axis*this->pos);
}
void SlideJoint::move(const float &offset)
{
	this->pos = glm::clamp(pos + offset, min, max);
	setOffset(axis*this->pos);
}