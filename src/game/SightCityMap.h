#pragma once
#include "core/Math.h"
#include "game/World.h"
#include <vector>
#include <string>
#include <cstdint>

enum class RoadClass : uint8_t { Local, Avenue, Highway, Rural };
struct MapRoadSegment { Vec3 a{}, b{}; RoadClass roadClass{RoadClass::Local}; float width{8.0f}; float speedLimit{13.0f}; };
struct MapChunk3D {
    int cellX{0}, cellZ{0}; RegionType region{RegionType::Countryside};
    std::vector<MapRoadSegment> roads; int buildingLots{0}; int vegetationLots{0};
};
class SightCityMap {
public:
    static constexpr int WorldMeters=120000;
    void stream(const Vec3& player,int radiusCells=3);
    const std::vector<MapChunk3D>& chunks() const { return chunks_; }
    std::vector<MapRoadSegment> nearbyRoads(const Vec3& p,float radius) const;
    Vec3 snapToRoad(const Vec3& p) const;
    std::size_t roadSegmentCount() const;
private:
    std::vector<MapChunk3D> chunks_;
    MapChunk3D buildChunk(int cx,int cz) const;
};
