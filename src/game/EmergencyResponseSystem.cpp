#include "game/EmergencyResponseSystem.h"
#include <algorithm>
#include <cmath>

static float d3(const Vec3&a,const Vec3&b){ return length(a-b); }
static float clamp01(float v){ return std::max(0.f,std::min(1.f,v)); }

EmergencyResponseSystem::EmergencyResponseSystem(bool vita){
    const int policeCount = vita ? 2 : 4;
    const int ambulanceCount = vita ? 1 : 3;
    const int fireCount = vita ? 1 : 2;
    int id=1;
    for(int i=0;i<policeCount;i++){
        Vec3 h{-90.f+i*12.f,0,-55.f};
        units_.push_back({id++,EmergencyService::Police,h,h,Vec3{},EmergencyUnitState::Available,-1,0,100,false,false,false});
    }
    for(int i=0;i<ambulanceCount;i++){
        Vec3 h{-35.f+i*12.f,0,-70.f};
        units_.push_back({id++,EmergencyService::Ambulance,h,h,Vec3{},EmergencyUnitState::Available,-1,0,100,false,false,false});
    }
    for(int i=0;i<fireCount;i++){
        Vec3 h{25.f+i*14.f,0,-70.f};
        units_.push_back({id++,EmergencyService::FireDepartment,h,h,Vec3{},EmergencyUnitState::Available,-1,0,100,false,false,false});
    }
}

int EmergencyResponseSystem::reportIncident(EmergencyIncidentType type,const Vec3&p,float severity,bool crime,bool fire){
    EmergencyIncident i;
    i.id=nextIncidentId_++; i.type=type; i.position=p; i.severity=clamp01(severity);
    i.crimeSuspected=crime; i.firePresent=fire || type==EmergencyIncidentType::Fire || type==EmergencyIncidentType::Explosion;
    incidents_.push_back(i);
    return i.id;
}

void EmergencyResponseSystem::reportFire(const Vec3&p,float severity){
    reportIncident(EmergencyIncidentType::Fire,p,severity,false,true);
}

int EmergencyResponseSystem::reportCasualty(const Vec3&p,float severity){
    Casualty c; c.id=nextCasualtyId_++; c.position=p; c.injurySeverity=clamp01(severity);
    casualties_.push_back(c); return c.id;
}

EmergencyIncident* EmergencyResponseSystem::findIncident(int id){
    for(auto& i:incidents_) if(i.id==id) return &i;
    return nullptr;
}

Casualty* EmergencyResponseSystem::findNearestUntreatedCasualty(const Vec3&p,float radius){
    Casualty* best=nullptr; float bd=radius;
    for(auto& c:casualties_){
        if(!c.alive || c.transported || c.stabilized) continue;
        float d=d3(c.position,p); if(d<bd){bd=d;best=&c;}
    }
    return best;
}

bool EmergencyResponseSystem::dispatchOne(EmergencyService service,EmergencyIncident& incident){
    for(auto& u:units_){
        if(u.service!=service || u.state!=EmergencyUnitState::Available) continue;
        u.target=incident.position; u.incidentId=incident.id; u.state=EmergencyUnitState::Dispatched;
        u.responseTimer=0.35f; u.siren=true; u.emergencyLights=true;
        if(service==EmergencyService::Police) ++incident.policeAssigned;
        else if(service==EmergencyService::Ambulance) ++incident.ambulanceAssigned;
        else ++incident.fireAssigned;
        return true;
    }
    return false;
}

void EmergencyResponseSystem::dispatchNeededUnits(EmergencyIncident& i){
    if(!i.active) return;
    int needPolice=0,needAmb=0,needFire=0;
    switch(i.type){
        case EmergencyIncidentType::MinorCrash: needPolice=1; needAmb=i.severity>=0.55f?1:0; break;
        case EmergencyIncidentType::MajorCrash: needPolice=1; needAmb=1; needFire=i.firePresent?1:0; break;
        case EmergencyIncidentType::Fire: needFire=1; needPolice=i.severity>=0.65f?1:0; needAmb=i.severity>=0.75f?1:0; break;
        case EmergencyIncidentType::Explosion: needPolice=1; needAmb=1; needFire=1; break;
        case EmergencyIncidentType::Shooting: needPolice=i.severity>=0.75f?2:1; needAmb=1; break;
        case EmergencyIncidentType::MedicalEmergency: needAmb=1; break;
        case EmergencyIncidentType::Crime: needPolice=1; needAmb=i.severity>=0.8f?1:0; break;
    }
    while(i.policeAssigned<needPolice && dispatchOne(EmergencyService::Police,i)){}
    while(i.ambulanceAssigned<needAmb && dispatchOne(EmergencyService::Ambulance,i)){}
    while(i.fireAssigned<needFire && dispatchOne(EmergencyService::FireDepartment,i)){}
}

bool EmergencyResponseSystem::incidentStillNeedsService(const EmergencyIncident& i,EmergencyService service) const{
    if(!i.active) return false;
    if(service==EmergencyService::FireDepartment) return i.firePresent;
    if(service==EmergencyService::Ambulance){
        for(const auto& c:casualties_) if(c.alive&&!c.transported&&d3(c.position,i.position)<30.f) return true;
        return false;
    }
    return i.crimeSuspected || i.type==EmergencyIncidentType::Shooting || i.type==EmergencyIncidentType::Crime || i.severity>=0.45f;
}

void EmergencyResponseSystem::returnUnit(EmergencyUnit& u){
    u.state=EmergencyUnitState::Returning; u.siren=false; u.emergencyLights=false; u.carryingPatient=false;
}

void EmergencyResponseSystem::updateUnit(EmergencyUnit& u,float dt,FireExplosionSystem& fire){
    EmergencyIncident* inc=findIncident(u.incidentId);
    if(u.state==EmergencyUnitState::Available) return;
    if(u.state==EmergencyUnitState::Dispatched){
        u.responseTimer-=dt; if(u.responseTimer<=0) u.state=EmergencyUnitState::EnRoute; return;
    }
    if(u.state==EmergencyUnitState::EnRoute){
        Vec3 v=u.target-u.position; float d=length(v); float speed=(u.service==EmergencyService::Police?24.f:20.f);
        if(d<4.f){
            u.siren=false;
            if(u.service==EmergencyService::Police) u.state=EmergencyUnitState::SecuringScene;
            else if(u.service==EmergencyService::Ambulance) u.state=EmergencyUnitState::Treating;
            else u.state=EmergencyUnitState::Suppressing;
            u.responseTimer=0;
        } else u.position += normalize(v)*std::min(d,speed*dt);
        return;
    }
    if(!inc){ returnUnit(u); return; }

    if(u.state==EmergencyUnitState::SecuringScene){
        u.responseTimer += dt;
        if(u.responseTimer>=3.f){
            ++securedScenes_;
            if(inc->crimeSuspected){
                u.state=EmergencyUnitState::Pursuing; u.responseTimer=0;
            } else returnUnit(u);
        }
        return;
    }
    if(u.state==EmergencyUnitState::Pursuing){
        u.responseTimer += dt; // prototype pursuit handoff; later connects directly to Wanted/Traffic AI
        if(u.responseTimer>=5.f){ inc->crimeSuspected=false; returnUnit(u); }
        return;
    }
    if(u.state==EmergencyUnitState::Treating){
        Casualty* c=findNearestUntreatedCasualty(inc->position,30.f);
        if(!c){ returnUnit(u); return; }
        u.target=c->position;
        float d=d3(u.position,c->position);
        if(d>2.2f){ u.position += normalize(c->position-u.position)*std::min(d,4.5f*dt); return; }
        u.responseTimer += dt; u.resource=std::max(0.f,u.resource-4.f*dt);
        if(u.responseTimer >= 1.5f + c->injurySeverity*2.f){
            c->stabilized=true; ++stabilized_; u.responseTimer=0; u.carryingPatient=true; u.state=EmergencyUnitState::Transporting;
            // Hospital anchor for current Downtown prototype.
            u.target=Vec3{70.f,0,-80.f};
        }
        return;
    }
    if(u.state==EmergencyUnitState::Transporting){
        Vec3 v=u.target-u.position; float d=length(v);
        if(d<3.f){
            for(auto& c:casualties_) if(c.stabilized&&!c.transported&&d3(c.position,inc->position)<30.f){ c.transported=true; ++transported_; break; }
            returnUnit(u);
        } else { u.siren=true; u.emergencyLights=true; u.position += normalize(v)*std::min(d,20.f*dt); }
        return;
    }
    if(u.state==EmergencyUnitState::Suppressing){
        u.resource=std::max(0.f,u.resource-12.f*dt); bool near=false;
        for(auto& f:const_cast<std::vector<FirePatch>&>(fire.fires())){
            if(f.active&&d3(f.position,inc->position)<14.f){
                near=true; f.intensity-=1.6f*dt;
                if(f.intensity<=.02f){ f.active=false; ++suppressed_; }
            }
        }
        if(!near || u.resource<=0){ inc->firePresent=false; returnUnit(u); }
        return;
    }
    if(u.state==EmergencyUnitState::Returning){
        Vec3 v=u.home-u.position; float d=length(v);
        if(d<3.f){
            // Release the assignment so the dispatcher can send this service again
            // when the same incident still has untreated casualties / fire / crime.
            if(EmergencyIncident* prev=findIncident(u.incidentId)){
                if(u.service==EmergencyService::Police) prev->policeAssigned=std::max(0,prev->policeAssigned-1);
                else if(u.service==EmergencyService::Ambulance) prev->ambulanceAssigned=std::max(0,prev->ambulanceAssigned-1);
                else prev->fireAssigned=std::max(0,prev->fireAssigned-1);
            }
            u.position=u.home; u.state=EmergencyUnitState::Available; u.incidentId=-1; u.resource=100; u.siren=false; u.emergencyLights=false;
        } else u.position += normalize(v)*std::min(d,18.f*dt);
    }
}

void EmergencyResponseSystem::update(float dt,FireExplosionSystem& fire){
    for(auto& i:incidents_) dispatchNeededUnits(i);
    for(auto& u:units_) updateUnit(u,dt,fire);
    for(auto& i:incidents_){
        if(!i.active) continue;
        bool unitsWorking=false;
        for(const auto& u:units_) if(u.incidentId==i.id && u.state!=EmergencyUnitState::Available && u.state!=EmergencyUnitState::Returning){ unitsWorking=true; break; }
        bool casualtyLeft=false;
        for(const auto& c:casualties_) if(c.alive&&!c.transported&&d3(c.position,i.position)<30.f){ casualtyLeft=true; break; }
        bool fireLeft=false;
        for(const auto& f:fire.fires()) if(f.active&&d3(f.position,i.position)<14.f){fireLeft=true;break;}
        if(!unitsWorking&&!casualtyLeft&&!fireLeft&&!i.crimeSuspected) i.active=false;
    }
    fire.update(dt);
}

void EmergencyResponseSystem::reactPedestrians(NPCSystem& npcs,const FireExplosionSystem& fire,float danger) const{
    for(auto& n:npcs.npcs){
        bool flee=false;
        for(const auto& f:fire.fires()) if(f.active&&d3(n.position,f.position)<danger){flee=true;break;}
        if(!flee) for(const auto& i:incidents_) if(i.active&&i.severity>=0.7f&&d3(n.position,i.position)<danger){flee=true;break;}
        if(flee){n.state=NPCState::Fleeing;n.stateTimer=4.f;}
    }
}

void EmergencyResponseSystem::applyTrafficYield(TrafficSystem& traffic,float radius){
    for(const auto& u:units_){
        if(!u.siren || (u.state!=EmergencyUnitState::EnRoute&&u.state!=EmergencyUnitState::Transporting)) continue;
        for(auto& c:traffic.cars){
            if(c.type==TrafficVehicleType::Police||c.type==TrafficVehicleType::Ambulance) continue;
            if(d3(c.position,u.position)<radius && c.speed>0.5f){
                c.aiState=TrafficAIState::Avoiding; c.speed*=0.70f; c.cruiseSpeed=std::max(4.f,c.cruiseSpeed*0.75f); ++trafficYieldEvents_;
            }
        }
    }
}

int EmergencyResponseSystem::activeResponses() const{
    int n=0; for(const auto& u:units_) if(u.state!=EmergencyUnitState::Available) ++n; return n;
}
int EmergencyResponseSystem::activeUnits(EmergencyService service) const{
    int n=0; for(const auto& u:units_) if(u.service==service&&u.state!=EmergencyUnitState::Available) ++n; return n;
}
