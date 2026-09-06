#pragma once
#include "core/Math.h"
#include <vector>
#include <cstdint>

enum class RegionType : uint8_t { Downtown, Urban, Suburb, Industrial, Airport, Countryside, Mountain, Coast, Ocean };

struct WorldCell {
    int x{}, z{};
    bool loaded{false};
    RegionType region{RegionType::Countryside};
    uint32_t seed{0};
};

class World {
public:
    static constexpr int CellSizeMeters = 64;
    static constexpr int WorldSizeMeters = 120000; // 120 km x 120 km
    static constexpr int HalfWorldMeters = WorldSizeMeters / 2;
    static constexpr int CellsPerAxis = WorldSizeMeters / CellSizeMeters; // 1875

    std::vector<WorldCell> cells;
    int radius{3}; // 7x7 nearby cells on Vita; world itself stays huge.

    void updateStreaming(const Vec3& playerPos);
    RegionType regionAt(int cellX, int cellZ) const;
    bool insideWorld(float x, float z) const;
    static const char* regionName(RegionType r);
};
