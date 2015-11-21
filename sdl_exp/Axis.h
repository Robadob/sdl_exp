#pragma once
class Axis
{
public:
    Axis(float length = 1.0);
    ~Axis();

    void render();

private:
    float length;
};

