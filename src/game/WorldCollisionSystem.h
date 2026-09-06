#pragma once
#include "core/Math.h"
#include "game/EnvironmentSystem.h"
#include "game/SightCityMap.h"
#include <vector>
#include <cstdint>

enum class CollisionKind : uint8_t { Tree, Rock, Cliff, Building, WorldBoundary, DeepWater };

struct CollisionHit {
    bool hit{false};
    CollisionKind kind{CollisionKind::Tree};
    Vec3 normal{};
    Vec3 point{};
};

struct BuildingCollider {
    Vec3 center{};
    Vec3 halfExtents{3.0f, 8.0f, 3.0f};
};

class WorldCollisionSystem {
public:
    float playerRadius{0.45f};
    float maxStepHeight{0.45f};
    float maxSlopeDegrees{42.0f};
    float waterDepthBlock{0.60f};

    void rebuild(const EnvironmentSystem& environment, const SightCityMap& map);
    Vec3 resolvePlayerMove(const Vec3& from, const Vec3& desired, const EnvironmentSystem& environment, CollisionHit* hit=nullptr) const;
    float groundHeight(float x, float z, const EnvironmentSystem& environment) const;
    float slopeDegrees(float x, float z, const EnvironmentSystem& environment) const;
    bool walkable(float x, float z, const EnvironmentSystem& environment) const;
    bool blockedAt(const Vec3& p, CollisionHit* hit=nullptr) const;
    const std::vector<BuildingCollider>& buildings() const { return buildings_; }

private:
    struct CircleCollider { Vec3 center{}; float radius{1.f}; CollisionKind kind{CollisionKind::Tree}; };
    std::vector<CircleCollider> circles_;
    std::vector<BuildingCollider> buildings_;
    bool blockedByBuildings(const Vec3& p, CollisionHit* hit) const;
    bool blockedByCircles(const Vec3& p, CollisionHit* hit) const;
};
