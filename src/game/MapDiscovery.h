#pragma once
#include "core/Math.h"
#include <cstdint>
#include <unordered_set>
#include <string>

enum class GameMode : uint8_t { Story=0, Online=1 };

class MapDiscovery {
public:
    static constexpr int TileSizeMeters = 256;
    void revealAround(const Vec3& pos, int radiusTiles=2);
    bool isRevealed(float worldX,float worldZ) const;
    std::size_t revealedCount() const { return revealed_.size(); }
    bool load(const std::string& path);
    bool save(const std::string& path) const;
private:
    std::unordered_set<uint64_t> revealed_;
    static uint64_t key(int x,int z);
};
