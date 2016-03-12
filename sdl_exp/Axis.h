#ifndef __Axis_h__
#define __Axis_h__

class Axis
{
public:
    Axis(float length = 1.0);
    ~Axis();

    void render();

private:
    float length;
};

#endif //ifndef __Axis_h__