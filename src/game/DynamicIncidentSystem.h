#pragma once
#include "core/Math.h"
#include <vector>
#include <cstdint>
enum class DynamicIncidentType:uint8_t { TrafficCrash, Robbery, KnifeAttack, Fight, Fire };
struct DynamicIncident { uint32_t id{0}; DynamicIncidentType type{DynamicIncidentType::TrafficCrash}; Vec3 position{}; };
class DynamicIncidentSystem {
public:
    DynamicIncident spawn(DynamicIncidentType type, const Vec3& playerPosition);
    bool recentlyUsed(const Vec3& p) const;
private:
    std::vector<Vec3> recent_;
    uint32_t nextId_{1}, rng_{0x51C17u};
};
