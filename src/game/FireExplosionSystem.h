#pragma once
#include "core/Math.h"
#include <vector>
#include <cstdint>

enum class IgnitionSource:uint8_t { Vehicle, Aircraft, Fuel, WorldObject };
struct ExplosionEvent { Vec3 position; float radius{8}; float peakDamage{100}; float impulse{25}; float age{0}; bool spawnedFire{true}; };
struct FirePatch { Vec3 position; float radius{2}; float intensity{1}; float lifetime{12}; float age{0}; IgnitionSource source{IgnitionSource::WorldObject}; bool active{true}; };

class FireExplosionSystem {
public:
    explicit FireExplosionSystem(bool vitaBudget=false);
    void explode(const Vec3& p,float radius,float damage,IgnitionSource source,bool ignite=true);
    void ignite(const Vec3& p,float radius,float intensity,float lifetime,IgnitionSource source);
    void update(float dt);
    float damageAt(const Vec3& p,float dt) const;
    float explosionDamageAt(const Vec3& p) const;
    const std::vector<ExplosionEvent>& explosions() const { return explosions_; }
    const std::vector<FirePatch>& fires() const { return fires_; }
    int activeFires() const;
    int smokeEmitterBudget() const { return smokeBudget_; }
private:
    std::vector<ExplosionEvent> explosions_;
    std::vector<FirePatch> fires_;
    int fireBudget_{32}; int explosionBudget_{12}; int smokeBudget_{24};
};
