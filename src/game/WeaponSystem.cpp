#include "game/WeaponSystem.h"
#include <algorithm>

OwnedWeapon* WeaponInventory::get(const std::string& id){ for(auto& w:owned) if(w.weaponId==id) return &w; return nullptr; }
const OwnedWeapon* WeaponInventory::get(const std::string& id) const { for(const auto& w:owned) if(w.weaponId==id) return &w; return nullptr; }
bool WeaponInventory::hasWeapon(const std::string& id) const { return get(id)!=nullptr; }
bool WeaponInventory::addWeapon(const WeaponDefinition& def){ if(hasWeapon(def.id)) return false; owned.push_back({def.id,def.magazineSize,false,false}); return true; }
bool WeaponInventory::addAmmo(const WeaponDefinition& def,int rounds){ auto*w=get(def.id); if(!w||rounds<=0) return false; w->ammo=std::min(9999,w->ammo+rounds); return true; }
bool WeaponInventory::setSight(const std::string& id,bool enabled){ auto*w=get(id); if(!w) return false; w->sight=enabled; return true; }
bool WeaponInventory::setSuppressor(const std::string& id,bool enabled){ auto*w=get(id); if(!w) return false; w->suppressor=enabled; return true; }
void WeaponInventory::setArmor(int value){ armorValue=std::max(0,std::min(100,value)); }
