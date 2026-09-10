#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/EnvironmentSystem.h"
#include "game/WorldCollisionSystem.h"
#include <cmath>
#include <algorithm>


// M106 explicit building collision -------------------------------------------
// This mirrors the deterministic M90/M98 building lot placement used by the
// Vita renderer. Open terrain stays walkable; only actual building footprints
// block Dash.
//
// Keeping this here avoids reusing WorldCollisionSystem::resolvePlayerMove(),
// which previously treated broad off-road areas as blocked.

static const float M106_CELL = 64.0f;
static const float M106_PLAYER_RADIUS = 0.34f;

struct M106Building {
    float x,z,sx,sz;
};

static unsigned m106Hash(int x,int z) {
    unsigned h=static_cast<unsigned>(x)*0x8da6b343u;
    h^=static_cast<unsigned>(z)*0xd8163841u;
    h^=(h>>13);
    h*=0x85ebca6bu;
    return h^(h>>16);
}

static int m106Region(float x,float z) {
    if(z < -42000.0f) return 0;
    if(z > 42000.0f && x > 10000.0f) return 1;
    if(z > 35000.0f && x < -15000.0f) return 2;
    if(x > 43000.0f || x < -43000.0f) return 3;
    if(z > 25000.0f) return 4;
    return 5;
}

static bool m106NearRoadX(int cx,float x) {
    if((cx%4)!=0) return false;
    const float rx=float(cx)*M106_CELL+32.0f;
    return std::fabs(x-rx)<8.0f;
}

static bool m106NearRoadZ(int cz,float z) {
    if((cz%4)!=0) return false;
    const float rz=float(cz)*M106_CELL+32.0f;
    return std::fabs(z-rz)<8.0f;
}

static bool m106OverlapPlaced(const M106Building* placed,int count,
                              float x,float z,float sx,float sz) {
    const float gap=4.0f;
    const float ahx=sx*0.5f;
    const float ahz=sz*0.5f;
    for(int i=0;i<count;i++) {
        const float bhx=placed[i].sx*0.5f;
        const float bhz=placed[i].sz*0.5f;
        if(std::fabs(x-placed[i].x) < ahx+bhx+gap &&
           std::fabs(z-placed[i].z) < ahz+bhz+gap)
            return true;
    }
    return false;
}

static int m106BuildingsForCell(int cx,int cz,M106Building out[4]) {
    const float x0=float(cx)*M106_CELL;
    const float z0=float(cz)*M106_CELL;
    const unsigned h=m106Hash(cx,cz);
    const int region=m106Region(x0+32.0f,z0+32.0f);

    unsigned density=0;
    if(region==5) density=2;
    else if(region==4 || region==3) density=((h>>4)&1u);
    else density=((h&7u)==0u)?1u:0u;

    static const float lotX[4]={12.0f,52.0f,12.0f,52.0f};
    static const float lotZ[4]={12.0f,12.0f,52.0f,52.0f};

    int count=0;
    for(unsigned i=0;i<density && i<4u;i++) {
        const unsigned q=m106Hash(cx*31+int(i)*17,cz*37+int(i)*23);
        const float sx=8.0f+float((q>>16)%4u);
        const float sz=8.0f+float((q>>20)%4u);

        const int li=int((q+i)%4u);
        float bx=x0+lotX[li]+(float((q>>5)%5u)-2.0f)*0.45f;
        float bz=z0+lotZ[li]+(float((q>>9)%5u)-2.0f)*0.45f;

        if(m106NearRoadX(cx,bx))
            bx=(bx<x0+32.0f)?x0+12.0f:x0+52.0f;
        if(m106NearRoadZ(cz,bz))
            bz=(bz<z0+32.0f)?z0+12.0f:z0+52.0f;

        if(m106OverlapPlaced(out,count,bx,bz,sx,sz))
            continue;

        out[count++]={bx,bz,sx,sz};
    }
    return count;
}

static bool m106HitsBuilding(float x,float z) {
    const int cx=int(std::floor(x/M106_CELL));
    const int cz=int(std::floor(z/M106_CELL));

    // Check neighboring cells too so collision stays correct on cell borders.
    for(int dz=-1;dz<=1;dz++) {
        for(int dx=-1;dx<=1;dx++) {
            M106Building buildings[4];
            const int count=m106BuildingsForCell(cx+dx,cz+dz,buildings);
            for(int i=0;i<count;i++) {
                const float hx=buildings[i].sx*0.5f+M106_PLAYER_RADIUS;
                const float hz=buildings[i].sz*0.5f+M106_PLAYER_RADIUS;
                if(std::fabs(x-buildings[i].x)<hx &&
                   std::fabs(z-buildings[i].z)<hz)
                    return true;
            }
        }
    }
    return false;
}

static Vec3 m106ResolveBuildings(const Vec3& from,const Vec3& desired) {
    Vec3 out=from;

    // Axis-separated collision gives natural wall sliding.
    if(!m106HitsBuilding(desired.x,from.z))
        out.x=desired.x;

    if(!m106HitsBuilding(out.x,desired.z))
        out.z=desired.z;

    return out;
}

void Player::updateState(const InputState& in,float mag,float speed){
    if(!grounded_) moveState_=verticalVelocity_>0?PlayerMoveState::Jump:PlayerMoveState::Fall;
    else if(mag<=0.05f) moveState_=PlayerMoveState::Idle;
    else if(in.stealth()) moveState_=PlayerMoveState::Stealth;
    else if(in.sprint()) moveState_=PlayerMoveState::Sprint;
    else if(speed>=3.5f) moveState_=PlayerMoveState::Run;
    else moveState_=PlayerMoveState::Walk;
}

void Player::update(const InputState& in,float dt){
    if(inVehicle){ moveState_=PlayerMoveState::Driving; return; }
    Vec3 wish{in.moveX,0,in.moveY}; float mag=length(wish); if(mag>1.0f) wish=normalize(wish);
    float speed=0.0f; if(mag>0.05f) speed=in.stealth()?2.0f:(in.sprint()?6.5f:(mag>0.65f?4.0f:2.7f));
    velocity.x=wish.x*speed; velocity.z=wish.z*speed; if(mag>0.05f) heading=std::atan2(wish.x,wish.z);
    if(grounded_ && in.jump()){ grounded_=false; verticalVelocity_=5.8f; fallStartY_=position.y; }
    if(!grounded_){ verticalVelocity_-=15.5f*dt; position.y+=verticalVelocity_*dt; if(position.y<=0.0f){ position.y=0.0f; verticalVelocity_=0.0f; grounded_=true; } }
    position.x+=velocity.x*dt; position.z+=velocity.z*dt; updateState(in,mag,speed);
}

void Player::updateWorld(const InputState& in,float dt,const EnvironmentSystem& environment,const WorldCollisionSystem& collisions){
    lastFallDamage_=0.0f;
    if(inVehicle){ moveState_=PlayerMoveState::Driving; return; }
    Vec3 wish{in.moveX,0,in.moveY}; float mag=length(wish); if(mag>1.0f) wish=normalize(wish);
    float speed=0.0f; if(mag>0.05f) speed=in.stealth()?2.0f:(in.sprint()?6.5f:(mag>0.65f?4.0f:2.7f));
    velocity.x=wish.x*speed; velocity.z=wish.z*speed; if(mag>0.05f) heading=std::atan2(wish.x,wish.z);

    const float ground=collisions.groundHeight(position.x,position.z,environment);
    if(grounded_) position.y=ground;
    if(grounded_ && in.jump()){ grounded_=false; verticalVelocity_=5.8f; fallStartY_=position.y; }

    Vec3 horizontalDesired{position.x+velocity.x*dt,position.y,position.z+velocity.z*dt};
    if(grounded_){
        // M105: the streamed collision prototype currently treats large off-road
        // areas as blocked. That is why Dash could walk on roads but got stuck
        // on grass. For the current Vita world prototype, allow horizontal
        // movement over terrain everywhere and use collision only for ground Y.
        //
        // Building/wall collision can be reintroduced later with explicit
        // obstacle volumes instead of using the whole world-cell resolver.
        // M106: grass/roads remain fully walkable, but actual deterministic
        // building footprints block the player.
        const Vec3 resolved=m106ResolveBuildings(position,horizontalDesired);
        position.x=resolved.x;
        position.z=resolved.z;

        // Keep Dash exactly on the terrain surface.
        position.y=collisions.groundHeight(position.x,position.z,environment);
        verticalVelocity_=0.0f;
    } else {
        // M105: keep air steering free over terrain too, while gravity/landing
        // still use the collision-ground height.
        const Vec3 airResolved=m106ResolveBuildings(position,horizontalDesired);
        position.x=airResolved.x;
        position.z=airResolved.z;
        verticalVelocity_-=15.5f*dt; position.y+=verticalVelocity_*dt;
        float landingGround=collisions.groundHeight(position.x,position.z,environment);
        if(position.y<=landingGround){
            float fallDistance=std::max(0.0f,fallStartY_-landingGround);
            if(fallDistance>4.0f){ lastFallDamage_=(fallDistance-4.0f)*7.5f; health=std::max(0.0f,health-lastFallDamage_); }
            position.y=landingGround;
            verticalVelocity_=0.0f;
            grounded_=true;
        }
    }

    // M71 safety clamp: when grounded, always snap exactly to terrain.
    if(grounded_){
        position.y=collisions.groundHeight(position.x,position.z,environment);
        verticalVelocity_=0.0f;
    }

    updateState(in,mag,speed);
}

bool Player::tryEnterVehicle(Vehicle& vehicle,float maxDistance){
    if(inVehicle) return true; Vec3 d=vehicle.position-position; d.y=0;
    if(length(d)>maxDistance) return false; inVehicle=true; moveState_=PlayerMoveState::Driving; syncWithVehicle(vehicle); return true;
}
void Player::syncWithVehicle(const Vehicle& vehicle){ if(inVehicle){ position=vehicle.position; heading=vehicle.heading; velocity={0,0,0}; } }
void Player::exitVehicle(Vehicle& vehicle){
    if(!inVehicle) return; inVehicle=false; grounded_=true; verticalVelocity_=0;
    position={vehicle.position.x+std::cos(vehicle.heading)*1.5f,vehicle.position.y,vehicle.position.z-std::sin(vehicle.heading)*1.5f};
    heading=vehicle.heading; moveState_=PlayerMoveState::Idle;
}
const char* Player::moveStateName() const{
    switch(moveState_){case PlayerMoveState::Idle:return "Idle";case PlayerMoveState::Walk:return "Walk";case PlayerMoveState::Run:return "Run";case PlayerMoveState::Sprint:return "Sprint";case PlayerMoveState::Stealth:return "Stealth";case PlayerMoveState::Jump:return "Jump";case PlayerMoveState::Fall:return "Fall";case PlayerMoveState::Driving:return "Driving";} return "Idle";
}
