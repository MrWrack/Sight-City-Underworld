#pragma once
#include <cmath>

struct Vec3 {
    float x{0}, y{0}, z{0};
    Vec3() = default;
    Vec3(float X,float Y,float Z):x(X),y(Y),z(Z){}
    Vec3 operator+(const Vec3& o) const { return {x+o.x,y+o.y,z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x,y-o.y,z-o.z}; }
    Vec3 operator*(float s) const { return {x*s,y*s,z*s}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
};

float length(const Vec3& v);
Vec3 normalize(const Vec3& v);
float clampf(float v,float lo,float hi);
