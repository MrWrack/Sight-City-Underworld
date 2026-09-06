#pragma once
#include "core/Math.h"
#include "game/Input.h"

class Vehicle;
class EnvironmentSystem;
class WorldCollisionSystem;
enum class PlayerMoveState : unsigned char { Idle, Walk, Run, Sprint, Stealth, Jump, Fall, Driving };

class Player {
public:
    Vec3 position{0,0,0};
    Vec3 velocity{0,0,0};
    float heading{0};
    bool inVehicle{false};
    float health{100.0f};

    void update(const InputState& in,float dt);
    void updateWorld(const InputState& in,float dt,const EnvironmentSystem& environment,const WorldCollisionSystem& collisions);
    bool tryEnterVehicle(Vehicle& vehicle,float maxDistance=3.0f);
    void exitVehicle(Vehicle& vehicle);
    void syncWithVehicle(const Vehicle& vehicle);
    PlayerMoveState moveState() const { return moveState_; }
    const char* moveStateName() const;
    bool grounded() const { return grounded_; }
    float lastFallDamage() const { return lastFallDamage_; }

private:
    bool grounded_{true};
    PlayerMoveState moveState_{PlayerMoveState::Idle};
    float verticalVelocity_{0.0f};
    float fallStartY_{0.0f};
    float lastFallDamage_{0.0f};
    void updateState(const InputState& in,float mag,float speed);
};
