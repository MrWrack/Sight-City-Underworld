#pragma once
#include "game/Camera.h"
#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/WantedSystem.h"
#include "game/World.h"
#include "game/Traffic.h"
#include "game/NPC.h"

class VitaRenderer {
public:
    bool init();
    void shutdown();
    void draw(const Player& player, const Vehicle& car, const Camera& camera,
              const World& world, const WantedSystem& wanted,
              const TrafficSystem& traffic, const NPCSystem& npcs, float fps);
};
