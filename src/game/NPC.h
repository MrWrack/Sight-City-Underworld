#pragma once
#include "core/Math.h"
#include "game/World.h"
#include <vector>
#include <cstdint>
enum class NPCState:uint8_t { Walking, Waiting, Crossing, Fleeing };
struct NPC { Vec3 position; float heading; float walkSpeed; NPCState state{NPCState::Walking}; float stateTimer{0}; };
class NPCSystem { public: std::vector<NPC> npcs; void update(const Vec3& player,float dt,RegionType region=RegionType::Downtown,bool danger=false); };
