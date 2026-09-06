#include "game/VehiclePhysics.h"
#include <cmath>
#include <algorithm>
static float len2(float x,float z){return std::sqrt(x*x+z*z);}
float VehiclePhysicsSystem::speed(const VehiclePhysicsBody& b) const{return len2(b.velocity.x,b.velocity.z);}
void VehiclePhysicsSystem::step(VehiclePhysicsBody& b,float dt,float grip) const{
    float s=speed(b); float engine=7.8f*b.throttle; float braking=12.0f*b.brake; float drag=0.018f*s*s+0.28f*s;
    float accel=engine-braking-drag/std::max(700.0f,b.mass)*1200.0f;
    s=std::max(0.0f,s+accel*dt);
    float steer=b.steering*0.58f; b.yawRate=(s/std::max(1.8f,b.wheelBase))*std::tan(steer)*grip;
    b.heading+=b.yawRate*dt;
    b.velocity.x=std::sin(b.heading)*s; b.velocity.z=std::cos(b.heading)*s;
    b.position.x+=b.velocity.x*dt; b.position.z+=b.velocity.z*dt;
}
bool VehiclePhysicsSystem::resolveVehicleCollision(VehiclePhysicsBody&a,VehiclePhysicsBody&b) const{
    float dx=b.position.x-a.position.x,dz=b.position.z-a.position.z,d=len2(dx,dz),minD=a.radius+b.radius;
    if(d>=minD||d<0.001f)return false; float nx=dx/d,nz=dz/d,overlap=minD-d;
    a.position.x-=nx*overlap*.5f;a.position.z-=nz*overlap*.5f;b.position.x+=nx*overlap*.5f;b.position.z+=nz*overlap*.5f;
    float av=a.velocity.x*nx+a.velocity.z*nz,bv=b.velocity.x*nx+b.velocity.z*nz,impact=std::fabs(av-bv);
    float impulse=(bv-av)*0.55f; a.velocity.x+=nx*impulse;a.velocity.z+=nz*impulse;b.velocity.x-=nx*impulse;b.velocity.z-=nz*impulse;
    if(impact>3.0f){a.crashed=b.crashed=true;a.damage=std::min(100.0f,a.damage+impact*1.4f);b.damage=std::min(100.0f,b.damage+impact*1.4f);} return true;
}
bool VehiclePhysicsSystem::resolveStaticCollision(VehiclePhysicsBody&b,const StaticRoadObstacle&o) const{
    float dx=b.position.x-o.position.x,dz=b.position.z-o.position.z,d=len2(dx,dz),minD=b.radius+o.radius;if(d>=minD||d<.001f)return false;
    float nx=dx/d,nz=dz/d,impact=std::fabs(b.velocity.x*nx+b.velocity.z*nz);b.position.x=o.position.x+nx*minD;b.position.z=o.position.z+nz*minD;
    b.velocity.x-=nx*(b.velocity.x*nx+b.velocity.z*nz)*1.35f;b.velocity.z-=nz*(b.velocity.x*nx+b.velocity.z*nz)*1.35f;b.crashed=true;b.damage=std::min(100.0f,b.damage+impact*2.0f);return true;
}
