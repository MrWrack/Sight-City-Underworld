#include "game/SightCityMap.h"
#include <cmath>
#include <limits>
static uint32_t hsh(int x,int z){ uint32_t h=(uint32_t)(x*73856093)^(uint32_t)(z*19349663); return h^(h>>13); }
static RegionType canonicalRegion(float x,float z){
    // Broad placement follows the locked Sight City layout; authored assets can replace geometry without changing coordinates.
    float d=std::sqrt(x*x+z*z);
    if(std::fabs(x)<5500&&z>-5000&&z<6500) return RegionType::Downtown;
    if(x>15000&&x<31000&&z>-15000&&z<3000) return RegionType::Airport;
    if(x<-28000&&z>-16000&&z<9000) return RegionType::Industrial;
    if(z>26000&&std::fabs(x)<28000) return RegionType::Mountain;
    if(d>50000) return RegionType::Coast;
    if(d<14000) return RegionType::Urban;
    if(d<24000) return RegionType::Suburb;
    return RegionType::Countryside;
}
MapChunk3D SightCityMap::buildChunk(int cx,int cz) const{
    MapChunk3D c; c.cellX=cx;c.cellZ=cz; float x=(cx+.5f)*World::CellSizeMeters,z=(cz+.5f)*World::CellSizeMeters;c.region=canonicalRegion(x,z);
    uint32_t s=hsh(cx,cz); float x0=cx*World::CellSizeMeters,z0=cz*World::CellSizeMeters,x1=x0+World::CellSizeMeters,z1=z0+World::CellSizeMeters;
    int stride=(c.region==RegionType::Downtown?2:(c.region==RegionType::Urban?3:(c.region==RegionType::Suburb?4:6)));
    if((cx%stride)==0){RoadClass rc=c.region==RegionType::Countryside?RoadClass::Rural:RoadClass::Avenue; c.roads.push_back({{x0,0,z0+32},{x1,0,z0+32},rc,rc==RoadClass::Rural?7.f:11.f,rc==RoadClass::Rural?20.f:14.f});}
    if((cz%stride)==0){RoadClass rc=c.region==RegionType::Countryside?RoadClass::Rural:RoadClass::Avenue; c.roads.push_back({{x0+32,0,z0},{x0+32,0,z1},rc,rc==RoadClass::Rural?7.f:11.f,rc==RoadClass::Rural?20.f:14.f});}
    // Sparse arterials keep long-distance routes legible while chunks remain Vita-sized.
    if(cx%16==0)c.roads.push_back({{x0+16,0,z0},{x0+16,0,z1},RoadClass::Highway,14.f,27.f});
    if(cz%16==0)c.roads.push_back({{x0,0,z0+16},{x1,0,z0+16},RoadClass::Highway,14.f,27.f});
    c.buildingLots=(c.region==RegionType::Downtown)?8:(c.region==RegionType::Urban?5:(c.region==RegionType::Suburb?3:(c.region==RegionType::Industrial?4:1)));
    c.vegetationLots=(c.region==RegionType::Countryside||c.region==RegionType::Mountain)?(3+(s%5)):(s%2); return c;
}
void SightCityMap::stream(const Vec3&p,int r){chunks_.clear();int cx=(int)std::floor(p.x/World::CellSizeMeters),cz=(int)std::floor(p.z/World::CellSizeMeters);chunks_.reserve((r*2+1)*(r*2+1));for(int z=-r;z<=r;z++)for(int x=-r;x<=r;x++){float wx=(cx+x+.5f)*World::CellSizeMeters,wz=(cz+z+.5f)*World::CellSizeMeters;if(wx>=-60000&&wx<60000&&wz>=-60000&&wz<60000)chunks_.push_back(buildChunk(cx+x,cz+z));}}
std::size_t SightCityMap::roadSegmentCount()const{size_t n=0;for(auto&c:chunks_)n+=c.roads.size();return n;}
std::vector<MapRoadSegment> SightCityMap::nearbyRoads(const Vec3&p,float r)const{std::vector<MapRoadSegment>o;float rr=r*r;for(auto&c:chunks_)for(auto&s:c.roads){Vec3 m={(s.a.x+s.b.x)*.5f,0,(s.a.z+s.b.z)*.5f};float dx=m.x-p.x,dz=m.z-p.z;if(dx*dx+dz*dz<=rr)o.push_back(s);}return o;}
Vec3 SightCityMap::snapToRoad(const Vec3&p)const{Vec3 best=p;float bd=std::numeric_limits<float>::max();for(auto&c:chunks_)for(auto&s:c.roads){Vec3 ab=s.b-s.a;float den=ab.x*ab.x+ab.z*ab.z;if(den<.01f)continue;float t=((p.x-s.a.x)*ab.x+(p.z-s.a.z)*ab.z)/den;t=t<0?0:(t>1?1:t);Vec3 q={s.a.x+ab.x*t,0,s.a.z+ab.z*t};float dx=q.x-p.x,dz=q.z-p.z,d=dx*dx+dz*dz;if(d<bd){bd=d;best=q;}}return best;}
