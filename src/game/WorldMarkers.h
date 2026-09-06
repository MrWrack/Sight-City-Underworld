#pragma once
#include "core/Math.h"
#include <vector>
#include <cstdint>
enum class MarkerType:uint8_t { SmallStore, GunStore, Supermarket, Mall, PropertyForSale, OwnedProperty, Garage, Bank, BusinessForSale, OwnedBusiness, BusinessMission, VehicleShop, Mission, Heist };
struct WorldMarker { uint32_t id; MarkerType type; Vec3 position; bool discoveredOnly; bool isNew=false; bool viewed=false; bool visited=false; bool story=true; bool online=true; };
class WorldMarkers { public: WorldMarkers(); std::vector<WorldMarker> markers; bool shouldPulse(uint32_t id)const; bool acknowledge(uint32_t id); bool visit(uint32_t id); };
