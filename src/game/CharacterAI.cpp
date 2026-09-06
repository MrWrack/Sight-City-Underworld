#include "game/CharacterAI.h"
#include <cmath>

CharacterAISystem::CharacterAISystem(){
    roster={
      {"dash","Dash",CharacterRole::Protagonist,{0,0,0},AIIntent::Hold,100,1,true,true},
      {"rhys","Rhys",CharacterRole::BestFriend,{-2,0,-2},AIIntent::Follow,100,0.85f,true,true},
      {"mayja","Mayja",CharacterRole::Planner,{3,0,-3},AIIntent::Plan,100,1,true,true},
      {"kane","Elias Kane",CharacterRole::Security,{5,0,-4},AIIntent::Hold,100,0.65f,true,true},
      {"vossler","Adrian Vossler",CharacterRole::Antagonist,{150,0,150},AIIntent::Plan,100,0,true,true}
    };
    for(int i=0;i<12;i++) roster.push_back({"crew_"+std::to_string(i+1),"Crew Member "+std::to_string(i+1),CharacterRole::Crew,{float((i%4)*2),0,float(5+(i/4)*2)},AIIntent::Follow,100,0.8f,true,true});
}
StoryCharacterAI* CharacterAISystem::find(const std::string&id){for(auto&c:roster)if(c.id==id)return &c;return nullptr;}
int CharacterAISystem::activeAllies() const {int n=0;for(const auto&c:roster)if(c.alive&&c.available&&c.role!=CharacterRole::Antagonist&&c.role!=CharacterRole::Protagonist)n++;return n;}
void CharacterAISystem::update(const Vec3&dash,float dt,bool combat){
    for(auto&c:roster){ if(!c.alive||!c.available||c.role==CharacterRole::Protagonist||c.role==CharacterRole::Antagonist) continue;
      if(c.role==CharacterRole::Planner){ c.intent=AIIntent::Plan; continue; }
      float dx=dash.x-c.position.x,dz=dash.z-c.position.z; float d=std::sqrt(dx*dx+dz*dz);
      if(combat) c.intent=(d<12?AIIntent::TakeCover:AIIntent::Fight); else c.intent=AIIntent::Follow;
      if(d>3.0f){float s=(combat?4.4f:2.4f)*dt; c.position.x += dx/(d+0.001f)*s; c.position.z += dz/(d+0.001f)*s;}
    }
}
