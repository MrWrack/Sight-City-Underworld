#include "core/Math.h"
float length(const Vec3& v){ return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z); }
Vec3 normalize(const Vec3& v){ float l=length(v); return l>0.0001f ? v*(1.0f/l) : Vec3{}; }
float clampf(float v,float lo,float hi){ return v<lo?lo:(v>hi?hi:v); }
