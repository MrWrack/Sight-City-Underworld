#include "game/FireExplosionSystem.h"
#include <algorithm>
#include <cmath>
static float dist3(const Vec3&a,const Vec3&b){ return length(a-b); }
FireExplosionSystem::FireExplosionSystem(bool vita){ if(vita){fireBudget_=16; explosionBudget_=6; smokeBudget_=12;} }
void FireExplosionSystem::explode(const Vec3&p,float r,float d,IgnitionSource s,bool igniteFire){
 if((int)explosions_.size()>=explosionBudget_) explosions_.erase(explosions_.begin());
 explosions_.push_back({p,r,d,r*3.f,0.f,igniteFire});
 if(igniteFire) ignite(p,std::max(1.5f,r*.35f),1.f,10.f+r*.4f,s);
}
void FireExplosionSystem::ignite(const Vec3&p,float r,float i,float life,IgnitionSource s){
 if((int)fires_.size()>=fireBudget_) fires_.erase(fires_.begin());
 fires_.push_back({p,r,i,life,0.f,s,true});
}
void FireExplosionSystem::update(float dt){
 for(auto&e:explosions_) e.age+=dt;
 explosions_.erase(std::remove_if(explosions_.begin(),explosions_.end(),[](const ExplosionEvent&e){return e.age>.35f;}),explosions_.end());
 for(auto&f:fires_){ f.age+=dt; f.intensity=std::max(0.f,f.intensity-dt*.018f); if(f.age>=f.lifetime||f.intensity<=.02f) f.active=false; }
 fires_.erase(std::remove_if(fires_.begin(),fires_.end(),[](const FirePatch&f){return !f.active;}),fires_.end());
}
float FireExplosionSystem::damageAt(const Vec3&p,float dt) const { float d=0; for(const auto&f:fires_) if(f.active){float x=dist3(p,f.position); if(x<f.radius) d+=(1.f-x/f.radius)*18.f*f.intensity*dt;} return d; }
float FireExplosionSystem::explosionDamageAt(const Vec3&p) const { float d=0; for(const auto&e:explosions_){float x=dist3(p,e.position); if(x<e.radius) d=std::max(d,(1.f-x/e.radius)*e.peakDamage);} return d; }
int FireExplosionSystem::activeFires() const { int n=0; for(const auto&f:fires_) if(f.active)++n; return n; }
