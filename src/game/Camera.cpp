#include "game/Camera.h"
#include "core/Math.h"
#include <cmath>
void Camera::cycle(bool firstPersonAllowed){
    if(mode==CameraMode::ThirdNear) mode=CameraMode::ThirdFar;
    else if(mode==CameraMode::ThirdFar) mode=firstPersonAllowed?CameraMode::FirstPerson:CameraMode::ThirdNear;
    else mode=CameraMode::ThirdNear;
}
void Camera::follow(const Vec3& focus,float heading,float lookX,float lookY,float dt,bool inVehicle){
    yaw += lookX*2.4f*dt;
    // M82: wider vertical camera tilt on PS Vita.
    // Allows looking substantially farther up and down.
    pitch=clampf(pitch+lookY*1.8f*dt,-1.15f,1.15f);
    const float a=heading+yaw;
    target=focus+Vec3{0,inVehicle?1.15f:1.55f,0};
    if(mode==CameraMode::FirstPerson){
        float eye=inVehicle?1.15f:1.62f;
        position={focus.x+std::sin(heading)*0.18f,focus.y+eye,focus.z+std::cos(heading)*0.18f};
        target={position.x+std::sin(a)*10.0f,position.y+pitch*5.0f,position.z+std::cos(a)*10.0f};
        return;
    }
    const float dist=mode==CameraMode::ThirdFar?9.5f:6.0f;
    position={target.x-std::sin(a)*dist,target.y+(mode==CameraMode::ThirdFar?3.4f:2.4f)+pitch*2.0f,target.z-std::cos(a)*dist};
}
