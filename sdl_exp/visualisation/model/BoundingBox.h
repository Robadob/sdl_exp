#ifndef __BoundingBox_h__
#define __BoundingBox_h__

#include <glm/glm.hpp>

template <class tvec>
class BoundingBoxt
{
    tvec minPt, maxPt;
public:
    BoundingBoxt()
        : minPt(0)
        , maxPt(0)
    { }

    BoundingBoxt& include(tvec t)
    {
        minPt = glm::min(minPt, t);
        maxPt = glm::max(maxPt, t);

        return *this;
    } 
    BoundingBoxt& include(BoundingBoxt t)
    {
        minPt = glm::min(minPt, t.min());
        maxPt = glm::max(maxPt, t.max());

        return *this;
    }
    bool contains(tvec t)
    {
        return glm::all(glm::lessThanEqual(t, maxPt)) && glm::all(glm::greaterThanEqual(t, minPt));
    }
    bool contains(BoundingBoxt t)
    {
        return glm::all(glm::lessThanEqual(t.max(), maxPt)) && glm::all(glm::greaterThanEqual(t.min(), minPt));
    }
    void reset()
    {
        minPt = tvec(0);
        maxPt = tvec(0);
    }
    tvec min() { return minPt; }
    tvec max() { return maxPt; }
    tvec size() { return maxPt - minPt; }
    tvec center() { return minPt+((maxPt - minPt) / 2.0f); }
};

typedef BoundingBoxt<float> BoundingBox1D;
typedef BoundingBoxt<glm::vec2> BoundingBox2D;
typedef BoundingBoxt<glm::vec3> BoundingBox3D;
typedef BoundingBoxt<glm::vec4> BoundingBox4D;

#endif