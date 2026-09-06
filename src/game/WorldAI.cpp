#include "game/WorldAI.h"
#include <cmath>
void WorldAI::update(const Vec3&player,float dt,RegionType region,bool danger){
    simTime+=dt; pedestrians.update(player,dt,region,danger); storyCharacters.update(player,dt,danger);
    for(auto& e:crimes){e.timer-=dt;} for(size_t i=0;i<crimes.size();){if(crimes[i].timer<=0) crimes.erase(crimes.begin()+i); else ++i;}
    if(!pedestrians.npcs.empty() && crimes.size()<3){
      int tick=(int)(simTime*0.2f); size_t idx=(size_t)((tick*17+3)%pedestrians.npcs.size());
      bool exists=false;for(const auto&e:crimes)if(e.npcIndex==idx){exists=true;break;}
      if(!exists && ((int)simTime)%19==0){
        NPCCrime c=(idx%5==0)?NPCCrime::ArmedRobbery:(idx%3==0?NPCCrime::Theft:NPCCrime::Speeding);
        float dx=pedestrians.npcs[idx].position.x-player.x,dz=pedestrians.npcs[idx].position.z-player.z;
        bool witnessed=(dx*dx+dz*dz)<6400.0f;
        crimes.push_back({idx,c,witnessed,witnessed,12.0f});
      }
    }
}
int WorldAI::policeResponses() const {int n=0;for(const auto&e:crimes)if(e.policeResponding)n++;return n;}
