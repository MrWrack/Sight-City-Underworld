#pragma once
#include "core/Math.h"
#include "game/World.h"
#include <vector>
#include <cstdint>

struct RoadNode {
    Vec3 position{};
    std::vector<int> links;
    float speedLimit{12.0f};
    bool intersection{false};
};

struct RoadRoute {
    std::vector<int> nodes;
    int cursor{0};
    bool valid() const { return !nodes.empty() && cursor < (int)nodes.size(); }
};

class RoadNetwork {
public:
    void buildAround(const Vec3& center, RegionType region);
    RoadRoute route(const Vec3& from,const Vec3& to) const;
    int nearestNode(const Vec3& p) const;
    const RoadNode* node(int index) const;
    std::size_t nodeCount() const { return nodes_.size(); }
private:
    std::vector<RoadNode> nodes_;
    Vec3 center_{};
    RegionType region_{RegionType::Downtown};
};
