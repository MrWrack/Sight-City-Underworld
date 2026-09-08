#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/EnvironmentSystem.h"
#include "game/WorldCollisionSystem.h"
#include <cmath>
#include <algorithm>

void Player::updateState(const InputState& in,float mag,float speed){
    if(!grounded_) moveState_=verticalVelocity_>0?PlayerMoveState::Jump:PlayerMoveState::Fall;
    else if(mag<=0.05f) moveState_=PlayerMoveState::Idle;
    else if(in.stealth()) moveState_=PlayerMoveState::Stealth;
    else if(in.sprint()) moveState_=PlayerMoveState::Sprint;
    else if(speed>=3.5f) moveState_=PlayerMoveState::Run;
    else moveState_=PlayerMoveState::Walk;
}

void Player::update(const InputState& in,float dt){
    if(inVehicle){ moveState_=PlayerMoveState::Driving; return; }
    Vec3 wish{in.moveX,0,in.moveY}; float mag=length(wish); if(mag>1.0f) wish=normalize(wish);
    float speed=0.0f; if(mag>0.05f) speed=in.stealth()?2.0f:(in.sprint()?6.5f:(mag>0.65f?4.0f:2.7f));
    velocity.x=wish.x*speed; velocity.z=wish.z*speed; if(mag>0.05f) heading=std::atan2(wish.x,wish.z);
    if(grounded_ && in.jump()){ grounded_=false; verticalVelocity_=5.8f; fallStartY_=position.y; }
    if(!grounded_){ verticalVelocity_-=15.5f*dt; position.y+=verticalVelocity_*dt; if(position.y<=0.0f){ position.y=0.0f; verticalVelocity_=0.0f; grounded_=true; } }
    position.x+=velocity.x*dt; position.z+=velocity.z*dt; updateState(in,mag,speed);
}

void Player::updateWorld(const InputState& in,float dt,const EnvironmentSystem& environment,const WorldCollisionSystem& collisions){
    lastFallDamage_=0.0f;
    if(inVehicle){ moveState_=PlayerMoveState::Driving; return; }
    Vec3 wish{in.moveX,0,in.moveY}; float mag=length(wish); if(mag>1.0f) wish=normalize(wish);
    float speed=0.0f; if(mag>0.05f) speed=in.stealth()?2.0f:(in.sprint()?6.5f:(mag>0.65f?4.0f:2.7f));
    velocity.x=wish.x*speed; velocity.z=wish.z*speed; if(mag>0.05f) heading=std::atan2(wish.x,wish.z);

    const float ground=collisions.groundHeight(position.x,position.z,environment);

    // M96 anti-hover recovery: if physics says Dash is airborne but he is only
    // a tiny amount above/below the collision floor, recover to grounded state.
    // Real jumps remain untouched once they are clearly above the floor.
    if(!grounded_ && std::fabs(position.y-ground)<0.35f && verticalVelocity_<=0.0f) {
        position.y=ground;
        verticalVelocity_=0.0f;
        grounded_=true;
    }
    if(grounded_) position.y=ground;
    if(grounded_ && in.jump()){ grounded_=false; verticalVelocity_=5.8f; fallStartY_=position.y; }

    Vec3 horizontalDesired{position.x+velocity.x*dt,position.y,position.z+velocity.z*dt};
    if(grounded_){
        Vec3 resolved=collisions.resolvePlayerMove(position,horizontalDesired,environment,nullptr);
        position.x=resolved.x;
        position.z=resolved.z;

        // M71: keep Dash locked to the actual terrain while grounded.
        // Do not trust a stale/resolved Y value from horizontal collision handling.
        position.y=collisions.groundHeight(position.x,position.z,environment);
        verticalVelocity_=0.0f;
    } else {
        // Air movement remains collision aware horizontally, but does not glue Dash to the ground.
        Vec3 probe=collisions.resolvePlayerMove({position.x,collisions.groundHeight(position.x,position.z,environment),position.z},
                                                 {horizontalDesired.x,0,horizontalDesired.z},environment,nullptr);
        position.x=probe.x; position.z=probe.z;
        verticalVelocity_-=15.5f*dt; position.y+=verticalVelocity_*dt;
        float landingGround=collisions.groundHeight(position.x,position.z,environment);
        if(position.y<=landingGround){
            float fallDistance=std::max(0.0f,fallStartY_-landingGround);
            if(fallDistance>4.0f){ lastFallDamage_=(fallDistance-4.0f)*7.5f; health=std::max(0.0f,health-lastFallDamage_); }
            position.y=landingGround;
            verticalVelocity_=0.0f;
            grounded_=true;
        }
    }

    // M71 safety clamp: when grounded, always snap exactly to terrain.
    if(grounded_){
        position.y=collisions.groundHeight(position.x,position.z,environment);
        verticalVelocity_=0.0f;
    }

    updateState(in,mag,speed);
}

bool Player::tryEnterVehicle(Vehicle& vehicle,float maxDistance){
    if(inVehicle) return true; Vec3 d=vehicle.position-position; d.y=0;
    if(length(d)>maxDistance) return false; inVehicle=true; moveState_=PlayerMoveState::Driving; syncWithVehicle(vehicle); return true;
}
void Player::syncWithVehicle(const Vehicle& vehicle){ if(inVehicle){ position=vehicle.position; heading=vehicle.heading; velocity={0,0,0}; } }
void Player::exitVehicle(Vehicle& vehicle){
    if(!inVehicle) return; inVehicle=false; grounded_=true; verticalVelocity_=0;
    position={vehicle.position.x+std::cos(vehicle.heading)*1.5f,vehicle.position.y,vehicle.position.z-std::sin(vehicle.heading)*1.5f};
    heading=vehicle.heading; moveState_=PlayerMoveState::Idle;
}
const char* Player::moveStateName() const{
    switch(moveState_){case PlayerMoveState::Idle:return "Idle";case PlayerMoveState::Walk:return "Walk";case PlayerMoveState::Run:return "Run";case PlayerMoveState::Sprint:return "Sprint";case PlayerMoveState::Stealth:return "Stealth";case PlayerMoveState::Jump:return "Jump";case PlayerMoveState::Fall:return "Fall";case PlayerMoveState::Driving:return "Driving";} return "Idle";
}
