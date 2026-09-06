#include "game/World.h"
#include <cmath>

namespace {
uint32_t cellHash(int x,int z){
    uint32_t h=(uint32_t)(x*374761393u) ^ (uint32_t)(z*668265263u);
    h=(h^(h>>13))*1274126177u; return h^(h>>16);
}
}

bool World::insideWorld(float x,float z) const {
    return x >= -HalfWorldMeters && x < HalfWorldMeters && z >= -HalfWorldMeters && z < HalfWorldMeters;
}

RegionType World::regionAt(int cx,int cz) const {
    const float x=(cx+0.5f)*CellSizeMeters;
    const float z=(cz+0.5f)*CellSizeMeters;
    const float d=std::sqrt(x*x+z*z);
    // Central Sight City metropolis. These are deterministic broad biomes; later milestones can replace
    // them with authored district data without changing the streaming API.
    if(std::fabs(x)<3200 && std::fabs(z)<3200) return RegionType::Downtown;
    if(d<8500) return RegionType::Urban;
    if(d<15000) return RegionType::Suburb;
    if(x>15000 && x<26000 && z>-8000 && z<5000) return RegionType::Airport;
    if(x<-12000 && x>-26000 && z>-12000 && z<4000) return RegionType::Industrial;
    if(d>50000) return RegionType::Coast;
    if(z>26000 && std::fabs(x)<26000) return RegionType::Mountain;
    return RegionType::Countryside;
}

void World::updateStreaming(const Vec3& p){
    int cx=(int)std::floor(p.x/CellSizeMeters);
    int cz=(int)std::floor(p.z/CellSizeMeters);
    cells.clear(); cells.reserve((radius*2+1)*(radius*2+1));
    for(int z=-radius;z<=radius;z++) for(int x=-radius;x<=radius;x++){
        int gx=cx+x, gz=cz+z;
        float wx=(gx+0.5f)*CellSizeMeters, wz=(gz+0.5f)*CellSizeMeters;
        if(!insideWorld(wx,wz)) continue;
        cells.push_back({gx,gz,true,regionAt(gx,gz),cellHash(gx,gz)});
    }
}

const char* World::regionName(RegionType r){
    switch(r){
        case RegionType::Downtown:return "Downtown"; case RegionType::Urban:return "Urban";
        case RegionType::Suburb:return "Suburb"; case RegionType::Industrial:return "Industrial";
        case RegionType::Airport:return "Airport"; case RegionType::Countryside:return "Countryside";
        case RegionType::Mountain:return "Mountain"; case RegionType::Coast:return "Coast";
        case RegionType::Ocean:return "Ocean";
    } return "Unknown";
}
