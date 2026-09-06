#include "game/CombatSystem.h"
#include <algorithm>
#include <cmath>

static float dist3(const Vec3&a,const Vec3&b){float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return std::sqrt(x*x+y*y+z*z);}
void CombatSystem::equip(WeaponInventory& inv,const WeaponDefinition& def){
    if(!inv.hasWeapon(def.id)) return; equippedWeapon_=def.id; magazineRounds_=std::min(def.magazineSize,inv.get(def.id)->ammo);
}
bool CombatSystem::reload(WeaponInventory& inv,const WeaponDefinition& def){
    auto*w=inv.get(def.id); if(!w||equippedWeapon_!=def.id||def.magazineSize<=0) return false;
    int need=def.magazineSize-magazineRounds_; int take=std::min(need,w->ammo); if(take<=0) return false;
    w->ammo-=take; magazineRounds_+=take; return true;
}
ShotResult CombatSystem::fire(WeaponInventory& inv,const WeaponDefinition& def,const Vec3& origin,const Vec3& aim,std::vector<CombatActor>& actors,bool witnessed){
    ShotResult r; if(equippedWeapon_!=def.id||def.weaponClass==WeaponClass::Melee||magazineRounds_<=0) return r;
    --magazineRounds_; r.fired=true; r.recoil=(def.weaponClass==WeaponClass::Shotgun?1.4f:def.weaponClass==WeaponClass::Sniper?1.2f:0.7f);
    CombatActor* best=nullptr; float bestD=1e9f;
    for(auto& a:actors){ if(!a.alive||a.faction==CombatFaction::Player) continue; float d=dist3(a.position,aim); if(d<bestD&&d<2.5f){best=&a;bestD=d;} }
    if(best){ float range=dist3(origin,best->position); r.headshot=(aim.y-best->position.y)>1.35f; float dmg=def.damage*(r.headshot?1.75f:1.0f); if(range>80.0f&&def.weaponClass!=WeaponClass::Sniper)dmg*=0.75f;
        float absorbed=std::min(best->armor,dmg*0.55f); best->armor-=absorbed; dmg-=absorbed; best->health=std::max(0.0f,best->health-dmg); if(best->health<=0){best->alive=false;best->intent=CombatIntent::Downed;} else best->intent=CombatIntent::Engage;
        r.hit=true;r.targetId=best->id;r.damage=dmg;
    }
    r.crimeReported=witnessed; return r;
}
void CombatSystem::updateAI(std::vector<CombatActor>& actors,const Vec3& player,bool armed,float dt){
    (void)dt; for(auto& a:actors){ if(!a.alive)continue; float d=dist3(a.position,player); if(a.faction==CombatFaction::Civilian){a.intent=(armed&&d<35)?CombatIntent::Flee:CombatIntent::Idle;} else if(a.faction==CombatFaction::Gang){a.intent=d<12?CombatIntent::TakeCover:(d<35?CombatIntent::Flank:CombatIntent::Engage);} else if(a.faction==CombatFaction::Police){a.intent=armed&&d<55?CombatIntent::Engage:CombatIntent::Idle;} }
}
float CombatSystem::applyPlayerDamage(float d){float absorb=std::min(playerArmor_,d*0.6f);playerArmor_-=absorb;return std::max(0.0f,d-absorb);}
