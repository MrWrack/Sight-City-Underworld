#pragma once
#include "core/Math.h"
#include "game/World.h"
#include <cstdint>
#include <vector>

enum class EnvironmentKind : uint8_t { Tree, Rock, GrassPatch, Cliff, Island, Beach, Water };
struct EnvironmentObject { EnvironmentKind kind{EnvironmentKind::GrassPatch}; Vec3 position{}; float scale{1.f}; bool collision{false}; uint8_t lod{0}; };
struct TerrainSample { float height{0.f}; bool water{false}; bool beach{false}; };

class EnvironmentSystem {
public:
    static constexpr float SeaLevel = 0.f;
    static constexpr float MountRidgePeakMeters = 1800.f;
    static constexpr float SouthHillsPeakMeters = 700.f;
    void stream(const Vec3& player, int radiusCells=3);
    TerrainSample sample(float x,float z) const;
    const std::vector<EnvironmentObject>& objects() const { return objects_; }
    int trees() const; int rocks() const; int grass() const; int cliffs() const; int islands() const;
    bool solidObstacleNear(const Vec3& p,float radius) const;
private:
    std::vector<EnvironmentObject> objects_;
    void buildCell(int cx,int cz,const Vec3& player);
};
