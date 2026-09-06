#pragma once
#include "core/Math.h"
#include "game/World.h"
#include <vector>
#include <cstdint>

enum class AnimalSpecies:uint8_t { Deer, Hare, Fox, Boar, Bird, Seagull };
enum class AnimalState:uint8_t { Roaming, Feeding, Resting, Fleeing };
struct Animal {
    AnimalSpecies species{AnimalSpecies::Deer};
    Vec3 position{};
    float heading{0};
    float speed{0};
    AnimalState state{AnimalState::Roaming};
    float stateTimer{0};
    bool alive{true};
};
class WildlifeSystem {
public:
    void update(const Vec3& player,float dt,RegionType region,bool danger=false);
    void reactToVehicle(const Vec3& vehicle,float speedMps,float dt);
    const std::vector<Animal>& animals() const { return active; }
    int vitaBudget() const { return 12; }
private:
    std::vector<Animal> active;
    uint32_t tick{1};
    int targetFor(RegionType r) const;
    AnimalSpecies speciesFor(RegionType r,int seed) const;
};
