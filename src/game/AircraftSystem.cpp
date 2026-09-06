#include "game/AircraftSystem.h"
#include <cmath>
#include <algorithm>
float Aircraft::speed() const { return length(velocity); }
void Aircraft::update(const InputState& in,float dt,float ground){
 if(state==AircraftState::Crashed) return;
 float steer=clampf(in.moveX,-1.f,1.f), elevator=clampf(in.moveY,-1.f,1.f);
 throttle=clampf(throttle + (in.accelerate()?0.55f:0.f)*dt - (in.braking()?0.75f:0.f)*dt,0.f,1.f);
 float sp=speed();
 if(!airborne()){
   state=sp>1.f?AircraftState::Taxi:AircraftState::Parked; heading += steer*(0.35f+0.35f*std::min(sp/25.f,1.f))*dt;
   float target=throttle*spec.maxSpeed*.45f; float accel=(target-sp)*.8f; sp=std::max(0.f,sp+accel*dt);
   velocity={std::sin(heading)*sp,0,std::cos(heading)*sp}; position+=velocity*dt; position.y=ground;
   if(sp>=spec.takeoffSpeed && elevator>0.15f){ state=AircraftState::Takeoff; pitch=.10f+.18f*elevator; velocity.y=sp*.12f; }
 } else {
   state=AircraftState::Flying; pitch=clampf(pitch+elevator*.45f*dt,-.35f,.45f); roll=clampf(roll+steer*.8f*dt,-.65f,.65f);
   heading += (roll*.55f+steer*.15f)*dt; float target=std::max(spec.takeoffSpeed*.75f,throttle*spec.maxSpeed); sp += (target-sp)*.35f*dt;
   float horiz=sp*std::cos(pitch); velocity={std::sin(heading)*horiz,std::sin(pitch)*sp,std::cos(heading)*horiz}; position+=velocity*dt;
   roll*=std::pow(.55f,dt); pitch*=std::pow(.94f,dt);
   if(position.y<=ground+1.0f){ if(sp<spec.takeoffSpeed*.85f && std::fabs(pitch)<.22f){position.y=ground;velocity.y=0;state=AircraftState::Taxi;} else {health=std::max(0.f,health-55.f); if(health<=0) state=AircraftState::Crashed; position.y=ground;} }
 }
}
AircraftSystem::AircraftSystem(){ catalog_={
 {"Skylet S20",AircraftClass::Light,28,105,18,1},{"Skylet S45",AircraftClass::Light,34,135,21,1},
 {"Aerofox XR",AircraftClass::Light,38,170,26,1},{"Vortex P9",AircraftClass::Jet,55,310,48,1},
 {"Velora J80",AircraftClass::Jet,62,360,55,1},{"Velora J900",AircraftClass::Airliner,72,285,52,1},
 {"Aircrest A220",AircraftClass::Airliner,68,270,48,1},{"Aircrest A700",AircraftClass::Airliner,75,290,55,1},
 {"Aircrest A900",AircraftClass::Airliner,78,300,58,1},{"Titan C40",AircraftClass::Cargo,60,220,50,1},
 {"Titan C90 Heavy",AircraftClass::Cargo,72,235,62,1},{"Sentinel T7",AircraftClass::Military,50,330,65,1},
 {"Sentinel F12",AircraftClass::Military,58,430,82,1},{"Sentinel F22X",AircraftClass::Military,62,510,95,1},
 {"BlazeFire 6",AircraftClass::Military,48,260,62,1},{"Marlin S8",AircraftClass::Light,32,145,23,1}}; }
Aircraft AircraftSystem::spawn(const std::string& name,const Vec3& pos) const { Aircraft a; for(const auto&s:catalog_) if(s.name==name){a.spec=s;break;} a.position=pos; return a; }
