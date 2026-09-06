#pragma once
#include "core/Math.h"
#include <vector>

struct VehiclePhysicsBody {
    Vec3 position{};
    Vec3 velocity{};
    float heading{0};
    float yawRate{0};
    float mass{1450};
    float wheelBase{2.65f};
    float radius{2.1f};
    float throttle{0};
    float brake{0};
    float steering{0};
    bool crashed{false};
    float damage{0};
};

struct StaticRoadObstacle { Vec3 position{}; float radius{2.0f}; };

class VehiclePhysicsSystem {
public:
    void step(VehiclePhysicsBody& body,float dt,float grip=1.0f) const;
    bool resolveVehicleCollision(VehiclePhysicsBody& a,VehiclePhysicsBody& b) const;
    bool resolveStaticCollision(VehiclePhysicsBody& body,const StaticRoadObstacle& obstacle) const;
    float speed(const VehiclePhysicsBody& body) const;
};
