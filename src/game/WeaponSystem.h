#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class WeaponClass : uint8_t { Handgun, SMG, Shotgun, Rifle, Sniper, Melee };

struct WeaponDefinition {
    std::string id;
    std::string name;
    WeaponClass weaponClass{WeaponClass::Handgun};
    int price{0};
    int ammoPrice{0};
    int ammoPack{0};
    int magazineSize{0};
    float damage{0.0f};
    bool supportsSight{false};
    bool supportsSuppressor{false};
};

struct OwnedWeapon {
    std::string weaponId;
    int ammo{0};
    bool sight{false};
    bool suppressor{false};
};

class WeaponInventory {
public:
    bool addWeapon(const WeaponDefinition& def);
    bool addAmmo(const WeaponDefinition& def, int rounds);
    bool setSight(const std::string& weaponId, bool enabled);
    bool setSuppressor(const std::string& weaponId, bool enabled);
    bool hasWeapon(const std::string& weaponId) const;
    OwnedWeapon* get(const std::string& weaponId);
    const OwnedWeapon* get(const std::string& weaponId) const;
    const std::vector<OwnedWeapon>& weapons() const { return owned; }
    int armor() const { return armorValue; }
    void setArmor(int value);
private:
    std::vector<OwnedWeapon> owned;
    int armorValue{0};
};
