#include "game/Traffic.h"
#include <cmath>
#include <algorithm>

static constexpr float PI=3.14159265358979323846f;
static int targetCountFor(RegionType r){
#ifdef PLATFORM_VITA
    switch(r){case RegionType::Downtown:return 18; case RegionType::Urban:return 15; case RegionType::Suburb:return 11; case RegionType::Industrial:return 10; case RegionType::Airport:return 9; case RegionType::Countryside:return 5; default:return 4;}
#else
    switch(r){case RegionType::Downtown:return 30; case RegionType::Urban:return 24; case RegionType::Suburb:return 18; case RegionType::Industrial:return 16; case RegionType::Airport:return 14; case RegionType::Countryside:return 8; default:return 6;}
#endif
}
static const char* models[]={"Vivo City","Korni GT","Bmme Sedan","Foorl Compact","Niisse Sport","Totola Family","Merc Executive","FGZ Coupe","Substa AWD","Kava Street","Vivo Trucks Cargo","Senia Hauler","Cove Cruiser","Fren Classic","Pol EV"};
static TrafficVehicleType regionalType(RegionType r,int i){
    if(i%17==0)return TrafficVehicleType::Police;
    if(r==RegionType::Industrial && i%4==0)return TrafficVehicleType::Truck;
    if(r==RegionType::Countryside && i%4==0)return TrafficVehicleType::OffRoad;
    if(r==RegionType::Airport && i%5==0)return TrafficVehicleType::Utility;
    if(r==RegionType::Downtown && i%7==0)return TrafficVehicleType::LuxuryCar;
    if(r==RegionType::Downtown && i%6==0)return TrafficVehicleType::Taxi;
    if(i%11==2)return TrafficVehicleType::SportsCar;
    if(i%13==4)return TrafficVehicleType::Bus;
    if(i%8==6)return TrafficVehicleType::Motorcycle;
    return TrafficVehicleType::Car;
}
static float angleTo(const Vec3&a,const Vec3&b){return std::atan2(b.x-a.x,b.z-a.z);}
static float wrapAngle(float a){while(a>PI)a-=2*PI;while(a<-PI)a+=2*PI;return a;}
static float dist(const Vec3&a,const Vec3&b){float x=a.x-b.x,z=a.z-b.z;return std::sqrt(x*x+z*z);}

void TrafficSystem::refreshRoute(TrafficCar& c,const Vec3& player){
    if(roads.nodeCount()==0)return;
    if(dist(c.destination,player)<30.0f || !c.route.valid()){
        int seed=(c.id*97 + (int)std::fabs(c.position.x*0.3f+c.position.z*0.7f));
        const RoadNode* n=roads.node(seed%(int)roads.nodeCount());
        if(n)c.destination=n->position;
        c.route=roads.route(c.position,c.destination);
    }
}
void TrafficSystem::ensurePopulation(const Vec3&p,RegionType region){
    if(roads.nodeCount()==0 || dist(roadCenter,p)>192.0f || roadRegion!=region){roads.buildAround(p,region);roadCenter=p;roadRegion=region;for(auto&c:cars)c.route={};lights.clear();}
    int target=targetCountFor(region);
    while((int)cars.size()<target){
        int i=(int)cars.size(); TrafficCar c; c.id=i; c.heading=(i&1)?0.0f:PI*0.5f;
        const RoadNode* rn=roads.node((i*7)%(int)std::max<std::size_t>(1,roads.nodeCount()));
        c.position=rn?rn->position:Vec3{p.x+(i%8-4)*28.0f,0,p.z+((i*3)%9-4)*32.0f};
        c.cruiseSpeed=(rn?rn->speedLimit:12.0f)*(0.75f+0.05f*(i%5)); c.speed=c.cruiseSpeed*.7f; c.type=regionalType(region,i); c.lane=i%2;
        c.model=models[i%(sizeof(models)/sizeof(models[0]))]; c.temperament=(DriverTemperament)(i%4);
        c.destination=p; c.lighting.lights=VehicleLightMode::LowBeam; c.physics.position=c.position; c.physics.heading=c.heading; c.physics.radius=c.collisionRadius; c.physics.velocity={std::sin(c.heading)*c.speed,0,std::cos(c.heading)*c.speed}; refreshRoute(c,p); cars.push_back(c);
    }
    if((int)cars.size()>target) cars.resize(target);
    if(lights.empty()) for(int z=-2;z<=2;z++) for(int x=-2;x<=2;x++) lights.push_back({{p.x+x*128.0f,0,p.z+z*128.0f},(float)((x+z+8)%3)*4.0f,TrafficSignal::Green,TrafficSignal::Red});
}
void TrafficSystem::updateLights(float dt){
    for(auto&l:lights){ l.phase+=dt; while(l.phase>=12.0f)l.phase-=12.0f;
        if(l.phase<5){l.northSouth=TrafficSignal::Green;l.eastWest=TrafficSignal::Red;} else if(l.phase<6){l.northSouth=TrafficSignal::Yellow;l.eastWest=TrafficSignal::Red;}
        else if(l.phase<11){l.northSouth=TrafficSignal::Red;l.eastWest=TrafficSignal::Green;} else {l.northSouth=TrafficSignal::Red;l.eastWest=TrafficSignal::Yellow;}}
}
void TrafficSystem::assignPoliceChases(){
    for(auto&c:cars){if(c.type==TrafficVehicleType::Police){c.chaseTarget=-1;if(policeAlert){float bd=99999;for(auto&t:cars)if(t.policeTarget&&t.type!=TrafficVehicleType::Police){float d=dist(c.position,t.position);if(d<bd){bd=d;c.chaseTarget=t.id;}}if(c.chaseTarget>=0)c.aiState=TrafficAIState::PolicePursuit;}}}
}
void TrafficSystem::updateDriver(size_t i,const Vec3&p,float dt,RegionType){
    auto& c=cars[i]; c.lighting.update(dt); c.decisionTimer-=dt; c.stateTimer-=dt;
    if(c.parked){ c.speed=0; c.aiState=TrafficAIState::Parked; if(c.stateTimer<=0){c.parked=false;c.aiState=TrafficAIState::Cruising;c.route={};} return; }
    refreshRoute(c,p);
    Vec3 target=c.destination;
    if(c.aiState==TrafficAIState::PolicePursuit && c.chaseTarget>=0 && c.chaseTarget<(int)cars.size())target=cars[c.chaseTarget].position;
    else if(c.route.valid()){
        const RoadNode* n=roads.node(c.route.nodes[c.route.cursor]); if(n){target=n->position;if(dist(c.position,target)<7.0f && c.route.cursor+1<(int)c.route.nodes.size())c.route.cursor++;}
    }
    float desiredHeading=angleTo(c.position,target), turn=wrapAngle(desiredHeading-c.heading); c.heading+=std::max(-1.8f*dt,std::min(1.8f*dt,turn));
    float nearestLight=9999; const TrafficLight* tl=nullptr; for(const auto&l:lights){float d=dist(l.position,c.position);if(d<nearestLight){nearestLight=d;tl=&l;}}
    bool ns=std::fabs(std::cos(c.heading))>0.7f; bool red=tl&&nearestLight<16&&((ns?tl->northSouth:tl->eastWest)==TrafficSignal::Red);
    float nearestCar=9999; bool danger=false; for(size_t j=0;j<cars.size();++j) if(j!=i){float dx=cars[j].position.x-c.position.x,dz=cars[j].position.z-c.position.z;float forward=dx*std::sin(c.heading)+dz*std::cos(c.heading);float side=std::fabs(dx*std::cos(c.heading)-dz*std::sin(c.heading));if(forward>0&&side<3.8f){nearestCar=std::min(nearestCar,forward);if(forward<5.5f)danger=true;}}
    if(policeAlert && c.type!=TrafficVehicleType::Police && c.temperament==DriverTemperament::Aggressive && (i%5==0)){c.aiState=TrafficAIState::FleeingPolice;c.policeTarget=true;c.cruiseSpeed=18.0f;}
    else if(c.type!=TrafficVehicleType::Police && red){c.aiState=TrafficAIState::StoppedAtLight;}
    else if(c.type!=TrafficVehicleType::Police && danger){c.aiState=TrafficAIState::Avoiding;collisionAvoidanceEvents++;}
    else if(c.type!=TrafficVehicleType::Police && nearestCar<12){c.aiState=TrafficAIState::Following;}
    else if(c.type!=TrafficVehicleType::Police && nearestCar<22 && c.temperament>=DriverTemperament::Impatient){c.aiState=TrafficAIState::Overtaking;c.laneOffset=(c.lane?-3.2f:3.2f);c.lighting.indicators=c.lane?IndicatorMode::Left:IndicatorMode::Right;}
    else if(c.aiState!=TrafficAIState::FleeingPolice && c.aiState!=TrafficAIState::PolicePursuit)c.aiState=TrafficAIState::Cruising;
    if(std::fabs(turn)>0.35f && c.type!=TrafficVehicleType::Police)c.lighting.indicators=turn<0?IndicatorMode::Left:IndicatorMode::Right; else if(std::fabs(turn)<0.12f)c.lighting.indicators=IndicatorMode::Off;
    if(c.decisionTimer<=0){c.decisionTimer=4.0f+(float)(i%4);if(c.temperament==DriverTemperament::Calm && ((i+(int)c.position.x/64)&15)==3 && c.stateTimer<=0){c.aiState=TrafficAIState::Parking;c.parked=true;c.stateTimer=3.0f+(i%5);}}
    float desired=c.cruiseSpeed; if(red && c.aiState!=TrafficAIState::PolicePursuit)desired=0; if(danger)desired=0; else if(nearestCar<12)desired=std::min(desired,4.0f); if(c.aiState==TrafficAIState::FleeingPolice)desired=19.0f; if(c.aiState==TrafficAIState::PolicePursuit)desired=20.0f;
    c.physics.position=c.position; c.physics.heading=c.heading; c.physics.radius=c.collisionRadius; c.physics.throttle=(desired>c.speed+0.3f)?1.0f:0.0f; c.physics.brake=(desired<c.speed-0.3f)?std::min(1.0f,(c.speed-desired)/8.0f):0.0f; c.physics.steering=std::max(-1.0f,std::min(1.0f,turn*1.7f)); if(c.speed>0.1f){c.physics.velocity.x=std::sin(c.heading)*c.speed;c.physics.velocity.z=std::cos(c.heading)*c.speed;} vehiclePhysics.step(c.physics,dt,1.0f); c.position=c.physics.position; c.heading=c.physics.heading; c.speed=vehiclePhysics.speed(c.physics); c.position.x+=std::cos(c.heading)*c.laneOffset*dt; c.position.z-=std::sin(c.heading)*c.laneOffset*dt; c.physics.position=c.position; c.laneOffset*=std::max(0.0f,1.0f-dt*1.8f);
    if(std::fabs(c.position.x-p.x)>520||std::fabs(c.position.z-p.z)>520){c.route={};c.destination=p;c.policeTarget=false;c.chaseTarget=-1;c.aiState=TrafficAIState::Cruising;c.parked=false;const RoadNode* n=roads.node((c.id*11)%(int)std::max<std::size_t>(1,roads.nodeCount()));if(n)c.position=n->position;}
}
void TrafficSystem::update(const Vec3&p,float dt,RegionType region){ensurePopulation(p,region);updateLights(dt);assignPoliceChases();for(size_t i=0;i<cars.size();++i)updateDriver(i,p,dt,region); for(size_t i=0;i<cars.size();++i)for(size_t j=i+1;j<cars.size();++j){cars[i].physics.position=cars[i].position;cars[j].physics.position=cars[j].position;cars[i].physics.radius=cars[i].collisionRadius;cars[j].physics.radius=cars[j].collisionRadius;if(vehiclePhysics.resolveVehicleCollision(cars[i].physics,cars[j].physics)){physicalCollisionEvents++;cars[i].position=cars[i].physics.position;cars[j].position=cars[j].physics.position;cars[i].speed=vehiclePhysics.speed(cars[i].physics);cars[j].speed=vehiclePhysics.speed(cars[j].physics);}}}
int TrafficSystem::fleeingDrivers() const {int n=0;for(const auto&c:cars)if(c.aiState==TrafficAIState::FleeingPolice)n++;return n;}
int TrafficSystem::policePursuits() const {int n=0;for(const auto&c:cars)if(c.aiState==TrafficAIState::PolicePursuit)n++;return n;}
