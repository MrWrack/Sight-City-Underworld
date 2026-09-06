#pragma once
#include "core/Math.h"
#include "game/Input.h"

enum class VehicleLightMode : unsigned char { Off=0, LowBeam=1, HighBeam=2 };
enum class IndicatorMode : unsigned char { Off=0, Left=1, Right=2, Hazards=3 };

struct VehicleLighting {
    VehicleLightMode lights{VehicleLightMode::Off};
    IndicatorMode indicators{IndicatorMode::Off};
    float blinkTimer{0};
    bool blinkVisible{false};
    void update(float dt);
    void cycleLights();
    void toggleLeft();
    void toggleRight();
    void toggleHazards();
};

class Vehicle {
public:
    Vec3 position{5,0,5};
    float heading{0};
    float speed{0};
    VehicleLighting lighting{};
    void update(const InputState& in,float dt,bool controlled,bool indicatorsEnabled=true,bool lightsEnabled=true);
};
