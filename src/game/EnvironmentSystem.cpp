#include "game/EnvironmentSystem.h"
#include <cmath>
constexpr float EnvironmentSystem::MountRidgePeakMeters;
constexpr float EnvironmentSystem::SouthHillsPeakMeters;
static uint32_t eh(int x,int z,uint32_t salt){uint32_t h=(uint32_t)(x*73856093)^(uint32_t)(z*19349663)^salt;h^=h>>13;h*=1274126177u;return h^(h>>16);}
static float rnd(uint32_t v){return (v&0xffffu)/65535.f;}
TerrainSample EnvironmentSystem::sample(float x,float z)const{
    // Locked Sight City mainland remains unchanged. Height layers only shape the terrain vertically.
    float radial=std::sqrt(x*x+z*z); const float coast=54000.f;
    float hills=18.f*std::sin(x*.00042f)+14.f*std::cos(z*.00037f)+8.f*std::sin((x+z)*.00091f);

    // Mount Ridge: broad foothills plus a localized summit. The summit is clamped to 1,800 m.
    const float ridgeX=-9000.f, ridgeZ=39000.f;
    float rdx=x-ridgeX, rdz=z-ridgeZ;
    float ridgeD2=rdx*rdx+rdz*rdz;
    float ridge=MountRidgePeakMeters*std::exp(-ridgeD2/(2.f*7200.f*7200.f));
    float northFoothills=0.f;
    if(z>22000.f) northFoothills=std::min(520.f,(z-22000.f)*.015f)*(.72f+.28f*std::sin(x*.00031f));

    // South Hills tops out around 700 m and rolls off smoothly toward the city/coast.
    const float southX=10000.f, southZ=-38500.f;
    float sdx=x-southX, sdz=z-southZ;
    float south=SouthHillsPeakMeters*std::exp(-(sdx*sdx+sdz*sdz)/(2.f*9000.f*9000.f));

    float island=0.f; // offshore islands: deterministic bumps, without changing the locked mainland layout.
    const float ix[4]={-56500.f,57000.f,-51000.f,53500.f}, iz[4]={18000.f,25000.f,-30000.f,-36000.f};
    for(int i=0;i<4;i++){float dx=x-ix[i],dz=z-iz[i],d=std::sqrt(dx*dx+dz*dz); if(d<2600.f)island=std::max(island,(2600.f-d)*.035f);}
    float mainland=coast-radial;
    float terrain = hills + northFoothills;
    if(ridge>35.f) terrain=std::max(terrain,ridge); // preserve the recognizable Mount Ridge massif.
    if(south>25.f) terrain=std::max(terrain,south);
    float h=(mainland>0? terrain+std::min(25.f,mainland*.025f) : -8.f)+island;
    if(ridgeD2 < 900.f*900.f) h=MountRidgePeakMeters; // summit plateau/sample guarantees the designed peak.
    h=std::min(h,MountRidgePeakMeters);
    bool water=h<=SeaLevel; bool beach=!water&&h<3.5f&&(radial>50000.f||island>0.f); return {h,water,beach};
}
void EnvironmentSystem::buildCell(int cx,int cz,const Vec3& player){
    float x0=cx*World::CellSizeMeters,z0=cz*World::CellSizeMeters; TerrainSample mid=sample(x0+32,z0+32); uint32_t s=eh(cx,cz,91);
    if(mid.water){ if((s%11)==0)objects_.push_back({EnvironmentKind::Water,{x0+32,0,z0+32},64.f,false,2}); return; }
    float dx=x0+32-player.x,dz=z0+32-player.z,dist=std::sqrt(dx*dx+dz*dz); uint8_t lod=dist<96?0:(dist<192?1:2);
    if(mid.beach)objects_.push_back({EnvironmentKind::Beach,{x0+32,mid.height,z0+32},32.f,false,lod});
    RegionType region=RegionType::Countryside; if(z0>22000)region=RegionType::Mountain; else if(std::sqrt((x0+32)*(x0+32)+(z0+32)*(z0+32))>50000)region=RegionType::Coast;
    int treeN=(region==RegionType::Mountain?5:(region==RegionType::Countryside?4:(region==RegionType::Coast?2:1)));
    int rockN=(region==RegionType::Mountain?4:1); int grassN=lod==0?6:(lod==1?3:1);
    for(int i=0;i<treeN;i++){float x=x0+6+rnd(eh(cx,cz,100+i))*52,z=z0+6+rnd(eh(cx,cz,200+i))*52;auto t=sample(x,z);if(!t.water)objects_.push_back({EnvironmentKind::Tree,{x,t.height,z},.8f+rnd(eh(cx,cz,300+i))*1.5f,true,lod});}
    for(int i=0;i<rockN;i++){float x=x0+4+rnd(eh(cx,cz,400+i))*56,z=z0+4+rnd(eh(cx,cz,500+i))*56;auto t=sample(x,z);if(!t.water)objects_.push_back({EnvironmentKind::Rock,{x,t.height,z},.6f+rnd(eh(cx,cz,600+i))*2.8f,true,lod});}
    for(int i=0;i<grassN;i++){float x=x0+rnd(eh(cx,cz,700+i))*64,z=z0+rnd(eh(cx,cz,800+i))*64;auto t=sample(x,z);if(!t.water&&!t.beach)objects_.push_back({EnvironmentKind::GrassPatch,{x,t.height,z},2.f+rnd(eh(cx,cz,900+i))*5.f,false,lod});}
    if(region==RegionType::Mountain&&(s%3)==0)objects_.push_back({EnvironmentKind::Cliff,{x0+32,mid.height,z0+32},4.f,true,lod});
    float r=std::sqrt((x0+32)*(x0+32)+(z0+32)*(z0+32)); if(r>54000&&mid.height>0)objects_.push_back({EnvironmentKind::Island,{x0+32,mid.height,z0+32},32.f,true,lod});
}
void EnvironmentSystem::stream(const Vec3&p,int r){objects_.clear();int cx=(int)std::floor(p.x/World::CellSizeMeters),cz=(int)std::floor(p.z/World::CellSizeMeters);for(int z=-r;z<=r;z++)for(int x=-r;x<=r;x++)buildCell(cx+x,cz+z,p);}
static int countKind(const std::vector<EnvironmentObject>&o,EnvironmentKind k){int n=0;for(auto&a:o)if(a.kind==k)n++;return n;}
int EnvironmentSystem::trees()const{return countKind(objects_,EnvironmentKind::Tree);} int EnvironmentSystem::rocks()const{return countKind(objects_,EnvironmentKind::Rock);} int EnvironmentSystem::grass()const{return countKind(objects_,EnvironmentKind::GrassPatch);} int EnvironmentSystem::cliffs()const{return countKind(objects_,EnvironmentKind::Cliff);} int EnvironmentSystem::islands()const{return countKind(objects_,EnvironmentKind::Island);}
bool EnvironmentSystem::solidObstacleNear(const Vec3&p,float radius)const{float rr=radius*radius;for(auto&o:objects_)if(o.collision){float dx=o.position.x-p.x,dz=o.position.z-p.z;if(dx*dx+dz*dz<rr)return true;}return false;}
