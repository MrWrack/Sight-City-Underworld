#include "platform/vita/VitaInput.h"
#include <cstring>
#include <cmath>

static float applyDeadzone(float v,float dz) {
    if (std::fabs(v) < dz) return 0.0f;
    const float s = v < 0.0f ? -1.0f : 1.0f;
    return s * (std::fabs(v) - dz) / (1.0f - dz);
}

float VitaInput::axis(unsigned char v,float dz) {
    float n=(static_cast<int>(v)-128)/127.0f;
    if(n<-1.0f)n=-1.0f; if(n>1.0f)n=1.0f;
    return applyDeadzone(n,dz);
}

VitaInput::VitaInput() {
    std::memset(&previous_,0,sizeof(previous_));
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
}

InputState VitaInput::poll(const GameSettings& settings,bool inVehicle) {
    SceCtrlData pad{};
    sceCtrlPeekBufferPositive(0,&pad,1);
    const unsigned int pressed=pad.buttons & ~previous_.buttons;

    InputState in{};
    // M67: standard PS Vita twin-stick layout.
    // Left stick = movement/steering. Right stick = camera/look.
    in.moveX=axis(pad.lx,settings.stickDeadzone);
    in.moveY=-axis(pad.ly,settings.stickDeadzone);

    // Horizontal camera direction corrected for the game's camera convention:
    // push right -> look right, push left -> look left.
    in.lookX=-axis(pad.rx,settings.stickDeadzone)*settings.lookSensitivity*(settings.invertCameraX?-1.0f:1.0f);

    // Vita Y axis grows downward, so negate it for normal camera controls:
    // push up -> look up, push down -> look down.
    in.lookY=-axis(pad.ry,settings.stickDeadzone)*settings.lookSensitivity*(settings.invertCameraY?-1.0f:1.0f);

    in.steer=in.moveX;

    auto bind=[&](InputAction a,unsigned int button){
        in.actions.set(a,(pad.buttons & button)!=0,(pressed & button)!=0);
    };

    // Shared/context actions.
    bind(InputAction::EnterExitVehicle,SCE_CTRL_TRIANGLE);
    bind(InputAction::Phone,SCE_CTRL_UP);

    if(!inVehicle) {
        bind(InputAction::Jump,SCE_CTRL_SQUARE);
        bind(InputAction::WeaponWheel,SCE_CTRL_LEFT);
        bind(InputAction::Sprint,SCE_CTRL_CROSS);
        bind(InputAction::Stealth,SCE_CTRL_CIRCLE);
        bind(InputAction::Aim,SCE_CTRL_LTRIGGER);
        bind(InputAction::Fire,SCE_CTRL_RTRIGGER);
        // Circle + Right is intentionally left free for a future remap UI.
        bind(InputAction::CameraCycle,SCE_CTRL_DOWN);
    } else {
        bind(InputAction::VehicleBrake,SCE_CTRL_CROSS);
        bind(InputAction::VehicleAccelerate,SCE_CTRL_RTRIGGER);
        bind(InputAction::VehicleIndicatorLeft,SCE_CTRL_LEFT);
        bind(InputAction::VehicleIndicatorRight,SCE_CTRL_RIGHT);
        bind(InputAction::VehicleHazards,SCE_CTRL_DOWN);
        bind(InputAction::VehicleLightsCycle,SCE_CTRL_LTRIGGER);
        bind(InputAction::CameraCycle,SCE_CTRL_SQUARE);
        in.throttle=in.accelerate()?1.0f:0.0f;
        in.brake=in.braking()?1.0f:0.0f;
    }

    if(pressed & SCE_CTRL_START) quit_=true;
    previous_=pad;
    return in;
}
