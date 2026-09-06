#pragma once
#include "core/Math.h"
#include "game/Input.h"
#include <string>
#include <vector>

enum class AircraftClass : unsigned char { Light, Jet, Airliner, Cargo, Military };
enum class AircraftState : unsigned char { Parked, Taxi, Takeoff, Flying, Landing, Crashed };
struct AircraftSpec { std::string name; AircraftClass type; float takeoffSpeed; float maxSpeed; float thrust; float lift; };

class Aircraft {
public:
    AircraftSpec spec{"Skylet S20",AircraftClass::Light,28.f,105.f,18.f,1.0f};
    Vec3 position{0,0,0}; Vec3 velocity{0,0,0};
    float heading{0}, pitch{0}, roll{0}, yawRate{0};
    float throttle{0}, health{100}; bool occupied{false}, gearDown{true};
    AircraftState state{AircraftState::Parked};
    void update(const InputState& in,float dt,float terrainHeight);
    float speed() const;
    bool airborne() const { return state==AircraftState::Flying || state==AircraftState::Landing; }
};

class AircraftSystem {
public:
    AircraftSystem();
    const std::vector<AircraftSpec>& catalog() const { return catalog_; }
    Aircraft spawn(const std::string& name,const Vec3& pos) const;
private: std::vector<AircraftSpec> catalog_;
};
