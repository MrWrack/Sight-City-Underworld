#include "game/Vehicle.h"
#include "core/Math.h"
#include <cmath>

void VehicleLighting::update(float dt){
    blinkTimer += dt;
    if(blinkTimer>=0.45f){ blinkTimer=0; blinkVisible=!blinkVisible; }
    if(indicators==IndicatorMode::Off) blinkVisible=false;
}
void VehicleLighting::cycleLights(){
    if(lights==VehicleLightMode::Off) lights=VehicleLightMode::LowBeam;
    else if(lights==VehicleLightMode::LowBeam) lights=VehicleLightMode::HighBeam;
    else lights=VehicleLightMode::Off;
}
void VehicleLighting::toggleLeft(){ indicators=indicators==IndicatorMode::Left?IndicatorMode::Off:IndicatorMode::Left; }
void VehicleLighting::toggleRight(){ indicators=indicators==IndicatorMode::Right?IndicatorMode::Off:IndicatorMode::Right; }
void VehicleLighting::toggleHazards(){ indicators=indicators==IndicatorMode::Hazards?IndicatorMode::Off:IndicatorMode::Hazards; }

void Vehicle::update(const InputState& in,float dt,bool controlled,bool indicatorsEnabled,bool lightsEnabled){
    lighting.update(dt);
    if(controlled){
        if(indicatorsEnabled){
            if(in.indicatorLeft()) lighting.toggleLeft();
            if(in.indicatorRight()) lighting.toggleRight();
            if(in.hazards()) lighting.toggleHazards();
        } else lighting.indicators=IndicatorMode::Off;
        if(lightsEnabled && in.lightsCycle()) lighting.cycleLights();
        if(!lightsEnabled) lighting.lights=VehicleLightMode::Off;
    }
    if(!controlled){ speed*=0.995f; return; }
    float accel=(in.accelerate()?1.0f:in.throttle) - in.brake;
    speed += accel*8.0f*dt;
    speed *= 0.992f;
    speed = clampf(speed,-4.0f,18.0f);
    heading += in.steer*1.8f*dt*(0.25f+std::abs(speed)/18.0f);
    position.x += std::sin(heading)*speed*dt;
    position.z += std::cos(heading)*speed*dt;
}
