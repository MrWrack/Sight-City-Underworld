#include "game/NPC.h"
#include <cmath>
#include <algorithm>
static int npcTarget(RegionType r){switch(r){case RegionType::Downtown:return 32;case RegionType::Urban:return 26;case RegionType::Suburb:return 18;case RegionType::Industrial:return 12;case RegionType::Airport:return 16;case RegionType::Countryside:return 6;default:return 4;}}
void NPCSystem::update(const Vec3&p,float dt,RegionType region,bool danger){
 int target=npcTarget(region); while((int)npcs.size()<target){int i=npcs.size();npcs.push_back({{p.x+(i%10-5)*11.0f,0,p.z+30+(i%6)*14.0f},(i%8)*0.7854f,1.0f+(i%3)*0.22f,NPCState::Walking,2.0f+(i%5)});} if((int)npcs.size()>target)npcs.resize(target);
 for(size_t i=0;i<npcs.size();++i){auto&n=npcs[i];float dx=n.position.x-p.x,dz=n.position.z-p.z;float d=std::sqrt(dx*dx+dz*dz);n.stateTimer-=dt;
   if(danger&&d<28.0f){n.state=NPCState::Fleeing;n.heading=std::atan2(dx,dz);n.stateTimer=3.0f;}
   else if(n.stateTimer<=0){int choice=((int)i*13+(int)std::fabs(n.position.x+n.position.z))%7;if(choice==0){n.state=NPCState::Waiting;n.stateTimer=1.0f+(i%3);}else{n.state=(choice==1?NPCState::Crossing:NPCState::Walking);n.heading+=(choice%3-1)*0.7854f;n.stateTimer=2.0f+(i%4);}}
   float s=n.state==NPCState::Waiting?0.0f:(n.state==NPCState::Fleeing?2.8f:n.walkSpeed); n.position.x+=std::sin(n.heading)*s*dt;n.position.z+=std::cos(n.heading)*s*dt;
   if(std::fabs(n.position.x-p.x)>260||std::fabs(n.position.z-p.z)>260){n.position.x=p.x+(float)((int)i%9-4)*18;n.position.z=p.z+80+(float)(i%5)*15;n.state=NPCState::Walking;}
 }
}
