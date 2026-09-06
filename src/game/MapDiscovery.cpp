#include "game/MapDiscovery.h"
#include <cmath>
#include <cstdio>
uint64_t MapDiscovery::key(int x,int z){ return (uint64_t)(uint32_t)x<<32 | (uint32_t)z; }
void MapDiscovery::revealAround(const Vec3& p,int r){
    int cx=(int)std::floor(p.x/TileSizeMeters), cz=(int)std::floor(p.z/TileSizeMeters);
    for(int z=-r;z<=r;++z) for(int x=-r;x<=r;++x) revealed_.insert(key(cx+x,cz+z));
}
bool MapDiscovery::isRevealed(float x,float z) const { return revealed_.count(key((int)std::floor(x/TileSizeMeters),(int)std::floor(z/TileSizeMeters)))!=0; }
bool MapDiscovery::save(const std::string& path) const { FILE* f=std::fopen(path.c_str(),"wb"); if(!f)return false; uint32_t n=(uint32_t)revealed_.size(); std::fwrite(&n,4,1,f); for(auto k:revealed_) std::fwrite(&k,8,1,f); std::fclose(f); return true; }
bool MapDiscovery::load(const std::string& path){ FILE* f=std::fopen(path.c_str(),"rb"); if(!f)return false; uint32_t n=0; if(std::fread(&n,4,1,f)!=1){std::fclose(f);return false;} revealed_.clear(); for(uint32_t i=0;i<n;i++){uint64_t k; if(std::fread(&k,8,1,f)!=1)break; revealed_.insert(k);} std::fclose(f); return true; }
