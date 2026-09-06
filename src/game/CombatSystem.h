#pragma once
#include "core/Math.h"
#include "game/WeaponSystem.h"
#include <string>
#include <vector>
#include <cstdint>

enum class CombatFaction : uint8_t { Player, Civilian, Gang, Police };
enum class CombatIntent : uint8_t { Idle, Engage, TakeCover, Flank, Flee, Downed };

struct CombatActor {
    int id{0};
    Vec3 position{0,0,0};
    float health{100.0f};
    float armor{0.0f};
    CombatFaction faction{CombatFaction::Civilian};
    CombatIntent intent{CombatIntent::Idle};
    bool alive{true};
};

struct ShotResult {
    bool fired{false};
    bool hit{false};
    bool headshot{false};
    int targetId{-1};
    float damage{0.0f};
    float recoil{0.0f};
    bool crimeReported{false};
};

class CombatSystem {
public:
    void equip(WeaponInventory& inventory,const WeaponDefinition& def);
    bool reload(WeaponInventory& inventory,const WeaponDefinition& def);
    ShotResult fire(WeaponInventory& inventory,const WeaponDefinition& def,const Vec3& origin,const Vec3& aimPoint,std::vector<CombatActor>& actors,bool witnessed);
    void updateAI(std::vector<CombatActor>& actors,const Vec3& playerPosition,bool playerArmed,float dt);
    void setPlayerArmor(float value){ playerArmor_=value<0?0:(value>100?100:value); }
    float applyPlayerDamage(float damage);
    float playerArmor() const { return playerArmor_; }
    int magazineRounds() const { return magazineRounds_; }
    const std::string& equippedWeapon() const { return equippedWeapon_; }
private:
    std::string equippedWeapon_;
    int magazineRounds_{0};
    float playerArmor_{0.0f};
};
