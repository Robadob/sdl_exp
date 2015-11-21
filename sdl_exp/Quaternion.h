#pragma once
class Quaternion
{
public:
    // Constructor(s)
    Quaternion();
    Quaternion(double x, double y, double z, double w);
    // Destructor
    ~Quaternion();

    // Public member variables for simple acces
    double x;
    double y;
    double z;
    double w;

    double length();
    void normalize();
    Quaternion conjugate();
    
    // Operators
    friend Quaternion operator*(const Quaternion &a, const Quaternion &b);
};

