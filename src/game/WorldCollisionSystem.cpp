#include "game/WorldCollisionSystem.h"
#include "game/World.h"
#include <cmath>
#include <algorithm>

namespace {
static uint32_t collisionHash(int x,int z,uint32_t salt){
    uint32_t h=(uint32_t)(x*73856093)^(uint32_t)(z*19349663)^salt;
    h^=h>>13; h*=1274126177u; return h^(h>>16);
}
static float hash01(uint32_t h){ return (h&0xffffu)/65535.f; }
static float clampf(float v,float a,float b){ return v<a?a:(v>b?b:v); }
}

void WorldCollisionSystem::rebuild(const EnvironmentSystem& environment, const SightCityMap& map){
    circles_.clear(); buildings_.clear();
    circles_.reserve(environment.objects().size());
    for(const auto& o:environment.objects()){
        if(!o.collision) continue;
        CollisionKind kind=CollisionKind::Rock;
        float radius=0.8f;
        switch(o.kind){
            case EnvironmentKind::Tree: kind=CollisionKind::Tree; radius=0.48f*o.scale+0.35f; break;
            case EnvironmentKind::Rock: kind=CollisionKind::Rock; radius=0.72f*o.scale+0.25f; break;
            case EnvironmentKind::Cliff: kind=CollisionKind::Cliff; radius=1.45f*o.scale; break;
            case EnvironmentKind::Island: kind=CollisionKind::Cliff; radius=0.0f; break; // terrain itself handles islands.
            default: radius=0.0f; break;
        }
        if(radius>0.0f) circles_.push_back({o.position,radius,kind});
    }

    // Cheap deterministic building collision volumes derived from streamed 64 m chunks.
    // Lots intentionally stay away from the chunk center cross so generated roads remain passable.
    for(const auto& c:map.chunks()){
        const float x0=c.cellX*World::CellSizeMeters;
        const float z0=c.cellZ*World::CellSizeMeters;
        for(int i=0;i<c.buildingLots;i++){
            uint32_t h=collisionHash(c.cellX,c.cellZ,1300u+(uint32_t)i*17u);
            int quadrant=i&3;
            float qx=(quadrant==0||quadrant==3)?-1.f:1.f;
            float qz=(quadrant<2)?-1.f:1.f;
            float ox=qx*(17.f+hash01(h)*9.f);
            float oz=qz*(17.f+hash01(h>>3)*9.f);
            float hx=2.8f+hash01(h>>7)*4.2f;
            float hz=2.8f+hash01(h>>11)*4.2f;
            float height=(c.region==RegionType::Downtown?18.f:(c.region==RegionType::Urban?12.f:7.f))+hash01(h>>5)*12.f;
            buildings_.push_back({{x0+32.f+ox,height*0.5f,z0+32.f+oz},{hx,height*0.5f,hz}});
        }
    }
}

float WorldCollisionSystem::groundHeight(float x,float z,const EnvironmentSystem& environment) const{
    return environment.sample(x,z).height;
}

float WorldCollisionSystem::slopeDegrees(float x,float z,const EnvironmentSystem& environment) const{
    const float e=0.75f;
    float hx0=environment.sample(x-e,z).height, hx1=environment.sample(x+e,z).height;
    float hz0=environment.sample(x,z-e).height, hz1=environment.sample(x,z+e).height;
    float gx=(hx1-hx0)/(2.f*e), gz=(hz1-hz0)/(2.f*e);
    float grad=std::sqrt(gx*gx+gz*gz);
    return std::atan(grad)*57.2957795f;
}

bool WorldCollisionSystem::walkable(float x,float z,const EnvironmentSystem& environment) const{
    if(x < -World::HalfWorldMeters+1.f || x > World::HalfWorldMeters-1.f || z < -World::HalfWorldMeters+1.f || z > World::HalfWorldMeters-1.f) return false;
    TerrainSample t=environment.sample(x,z);
    if(t.water && t.height < EnvironmentSystem::SeaLevel-waterDepthBlock) return false;
    return slopeDegrees(x,z,environment)<=maxSlopeDegrees;
}

bool WorldCollisionSystem::blockedByCircles(const Vec3& p,CollisionHit* hit) const{
    for(const auto& c:circles_){
        float dx=p.x-c.center.x,dz=p.z-c.center.z;
        float r=playerRadius+c.radius;
        float d2=dx*dx+dz*dz;
        if(d2<r*r){
            if(hit){
                float d=std::sqrt(std::max(d2,0.0001f));
                hit->hit=true; hit->kind=c.kind; hit->normal={dx/d,0,dz/d}; hit->point=p;
            }
            return true;
        }
    }
    return false;
}

bool WorldCollisionSystem::blockedByBuildings(const Vec3& p,CollisionHit* hit) const{
    for(const auto& b:buildings_){
        float dx=p.x-b.center.x,dz=p.z-b.center.z;
        float ex=b.halfExtents.x+playerRadius, ez=b.halfExtents.z+playerRadius;
        if(std::fabs(dx)<ex && std::fabs(dz)<ez){
            if(hit){
                hit->hit=true; hit->kind=CollisionKind::Building; hit->point=p;
                float px=ex-std::fabs(dx), pz=ez-std::fabs(dz);
                if(px<pz) hit->normal={(dx<0?-1.f:1.f),0,0}; else hit->normal={0,0,(dz<0?-1.f:1.f)};
            }
            return true;
        }
    }
    return false;
}

bool WorldCollisionSystem::blockedAt(const Vec3& p,CollisionHit* hit) const{
    CollisionHit local{};
    CollisionHit* out=hit?hit:&local;
    out->hit=false;
    return blockedByCircles(p,out)||blockedByBuildings(p,out);
}

Vec3 WorldCollisionSystem::resolvePlayerMove(const Vec3& from,const Vec3& desired,const EnvironmentSystem& environment,CollisionHit* hit) const{
    if(hit) hit->hit=false;
    Vec3 result=desired;
    if(!walkable(desired.x,desired.z,environment)){
        // Try axis sliding before giving up completely.
        Vec3 xOnly{desired.x,from.y,from.z};
        Vec3 zOnly{from.x,from.y,desired.z};
        if(walkable(xOnly.x,xOnly.z,environment) && !blockedAt(xOnly,hit)) result=xOnly;
        else if(walkable(zOnly.x,zOnly.z,environment) && !blockedAt(zOnly,hit)) result=zOnly;
        else { if(hit&&!hit->hit){hit->hit=true;hit->kind=CollisionKind::WorldBoundary;hit->point=from;} return from; }
    }
    CollisionHit obstacle{};
    if(blockedAt(result,&obstacle)){
        Vec3 xOnly{result.x,from.y,from.z}; Vec3 zOnly{from.x,from.y,result.z};
        if(!blockedAt(xOnly,nullptr)&&walkable(xOnly.x,xOnly.z,environment)) result=xOnly;
        else if(!blockedAt(zOnly,nullptr)&&walkable(zOnly.x,zOnly.z,environment)) result=zOnly;
        else { if(hit)*hit=obstacle; return from; }
        if(hit)*hit=obstacle;
    }
    float fromGround=groundHeight(from.x,from.z,environment);
    float toGround=groundHeight(result.x,result.z,environment);
    if(toGround-fromGround>maxStepHeight){ if(hit){hit->hit=true;hit->kind=CollisionKind::Cliff;hit->point=from;} return from; }
    result.y=toGround;
    return result;
}
