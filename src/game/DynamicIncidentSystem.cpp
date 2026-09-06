#include "DynamicIncidentSystem.h"
#include <cmath>
static float d2(const Vec3&a,const Vec3&b){float x=a.x-b.x,z=a.z-b.z;return x*x+z*z;}
bool DynamicIncidentSystem::recentlyUsed(const Vec3&p)const{for(const auto&r:recent_)if(d2(r,p)<250.f*250.f)return true;return false;}
DynamicIncident DynamicIncidentSystem::spawn(DynamicIncidentType type,const Vec3&player){
    Vec3 p=player;
    for(int tries=0;tries<24;++tries){
        rng_=1664525u*rng_+1013904223u; float a=float(rng_%6283u)/1000.f;
        rng_=1664525u*rng_+1013904223u; float radius=180.f+float(rng_%1800u);
        p={player.x+std::cos(a)*radius,0,player.z+std::sin(a)*radius};
        if(!recentlyUsed(p)) break;
    }
    recent_.push_back(p); if(recent_.size()>12) recent_.erase(recent_.begin());
    return {nextId_++,type,p};
}
