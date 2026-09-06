#include "game/GunStoreSystem.h"
#include <cstdio>

GunStoreSystem::GunStoreSystem(){
    weapons = {
        {"sidearm_9","9mm Sidearm",WeaponClass::Handgun,650,80,24,12,24.0f,true,true},
        {"heavy_pistol","Heavy Pistol",WeaponClass::Handgun,1200,100,24,12,38.0f,true,true},
        {"machine_pistol","Machine Pistol",WeaponClass::Handgun,1800,150,48,24,20.0f,true,true},
        {"compact_smg","Compact SMG",WeaponClass::SMG,2200,180,60,30,18.0f,true,true},
        {"tactical_smg","Tactical SMG",WeaponClass::SMG,3200,200,72,36,21.0f,true,true},
        {"pump_12","12G Pump Shotgun",WeaponClass::Shotgun,2600,160,20,8,62.0f,false,false},
        {"combat_shotgun","Combat Shotgun",WeaponClass::Shotgun,4200,200,24,12,55.0f,true,false},
        {"carbine_r","Carbine Rifle",WeaponClass::Rifle,4800,260,90,30,34.0f,true,true},
        {"assault_r","Assault Rifle",WeaponClass::Rifle,5600,280,90,30,39.0f,true,true},
        {"battle_r","Battle Rifle",WeaponClass::Rifle,6800,300,60,20,49.0f,true,true},
        {"marksman_s","Marksman Rifle",WeaponClass::Sniper,7200,320,30,10,82.0f,true,true},
        {"precision_s","Precision Rifle",WeaponClass::Sniper,9800,420,20,5,110.0f,true,true},
        {"knife","Combat Knife",WeaponClass::Melee,350,0,0,0,35.0f,false,false},
        {"baton","Heavy Baton",WeaponClass::Melee,300,0,0,0,28.0f,false,false}
    };
    for(const auto&w:weapons){
        items.push_back({"weapon:"+w.id,w.name,GunStoreItemType::Weapon,w.id,w.price,1});
        items.push_back({"ammo:"+w.id,w.name+" Ammo",GunStoreItemType::Ammo,w.id,w.ammoPrice,w.ammoPack});
        if(w.supportsSight) items.push_back({"sight:"+w.id,w.name+" Sight",GunStoreItemType::Sight,w.id,450,1});
        if(w.supportsSuppressor) items.push_back({"suppressor:"+w.id,w.name+" Suppressor",GunStoreItemType::Suppressor,w.id,700,1});
    }
    items.push_back({"armor:light","Light Body Armor",GunStoreItemType::Armor,"",500,50});
    items.push_back({"armor:heavy","Heavy Body Armor",GunStoreItemType::Armor,"",1200,100});
}

const WeaponDefinition* GunStoreSystem::weapon(const std::string&id) const { for(const auto&w:weapons) if(w.id==id) return &w; return nullptr; }

bool GunStoreSystem::buy(const std::string& itemId,WeaponInventory& inv,int& money,std::string& receipt){
    const GunStoreItem* item=nullptr; for(const auto&i:items) if(i.id==itemId){item=&i;break;} if(!item||money<item->price) return false;
    const WeaponDefinition* def=item->weaponId.empty()?nullptr:weapon(item->weaponId);
    bool ok=false;
    switch(item->type){
        case GunStoreItemType::Weapon: ok=def&&inv.addWeapon(*def); break;
        case GunStoreItemType::Ammo: ok=def&&inv.addAmmo(*def,item->quantity); break;
        case GunStoreItemType::Armor: inv.setArmor(item->quantity); ok=true; break;
        case GunStoreItemType::Sight: if(def&&def->supportsSight&&inv.hasWeapon(def->id)) ok=inv.setSight(def->id,true); break;
        case GunStoreItemType::Suppressor: if(def&&def->supportsSuppressor&&inv.hasWeapon(def->id)) ok=inv.setSuppressor(def->id,true); break;
    }
    if(!ok) return false;
    money-=item->price;
    char b[192]; std::snprintf(b,sizeof(b),"Purchased %s for $%d",item->displayName.c_str(),item->price); receipt=b;
    return true;
}
