#ifndef __SlideJoint_h__
#define __SlideJoint_h__
#include "SceneGraphJoint.h"

class SlideJoint : public SceneGraphJoint
{
public:
	static std::shared_ptr<SlideJoint> makeX(const float &min = 0, const float &max = 0, const float &init = 0);
	static std::shared_ptr<SlideJoint> makeY(const float &min = 0, const float &max = 0, const float &init = 0);
	static std::shared_ptr<SlideJoint> makeZ(const float &min = 0, const float &max = 0, const float &init = 0);
	void set(const float &offset);
	void move(const float &offset);
	void setMin(const float &min) { this->min = min; assert(max >= min); set(pos); };
	void setMax(const float &max) { this->max = max; assert(max >= min); set(pos); };
	float getMin() const { return min; }
	float getMax() const { return max; }
	float getOffset() const { return pos; }
private:
	SlideJoint(const glm::vec3 &axis, const float &min, const float &max, const float &init);
	float pos;
	const glm::vec3 axis;
	float min, max;
};
#endif //__SlideJoint_h__