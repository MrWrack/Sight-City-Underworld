#pragma once
#include "game/WeaponSystem.h"
#include <string>
#include <vector>
#include <cstdint>

enum class GunStoreItemType : uint8_t { Weapon, Ammo, Armor, Sight, Suppressor };
struct GunStoreItem {
    std::string id;
    std::string displayName;
    GunStoreItemType type{GunStoreItemType::Weapon};
    std::string weaponId;
    int price{0};
    int quantity{0};
};

class GunStoreSystem {
public:
    GunStoreSystem();
    const std::vector<WeaponDefinition>& weaponCatalog() const { return weapons; }
    const std::vector<GunStoreItem>& stock() const { return items; }
    const WeaponDefinition* weapon(const std::string& id) const;
    bool buy(const std::string& itemId, WeaponInventory& inventory, int& money, std::string& receipt);
private:
    std::vector<WeaponDefinition> weapons;
    std::vector<GunStoreItem> items;
};
