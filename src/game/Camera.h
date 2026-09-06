#pragma once
#include "core/Math.h"

enum class CameraMode : unsigned char { ThirdNear=0, ThirdFar=1, FirstPerson=2 };
class Camera {
public:
    Vec3 position{0,3,-6};
    Vec3 target{0,1,0};
    float yaw{0};
    float pitch{0.2f};
    CameraMode mode{CameraMode::ThirdNear};
    void cycle(bool firstPersonAllowed);
    void follow(const Vec3& focus,float heading,float lookX,float lookY,float dt,bool inVehicle=false);
};
