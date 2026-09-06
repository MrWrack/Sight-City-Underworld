#include "StoryHeistSystem.h"
void StoryHeistSystem::reset(){ plan_=StoryHeistPlan{}; }
bool StoryHeistSystem::meetMayja(){ plan_.mayjaMet=true; return true; }
bool StoryHeistSystem::selectGetawayVehicle(const std::string&id,bool owned,bool compatible,bool locked){ if(!plan_.mayjaMet||!owned||!compatible||locked||id.empty())return false; plan_.getawayVehicle=id; return true; }
bool StoryHeistSystem::collectEquipment(){ if(plan_.getawayVehicle.empty())return false; plan_.equipmentReady=true; return true; }
bool StoryHeistSystem::chooseEscapeRoute(EscapeRoute route){ if(!plan_.equipmentReady||route==EscapeRoute::None)return false; plan_.escapeRoute=route; return true; }
bool StoryHeistSystem::takeScore(){ if(!plan_.ready())return false; plan_.scoreTaken=true; return true; }
bool StoryHeistSystem::markEscaped(){ if(!plan_.scoreTaken)return false; plan_.escaped=true; return true; }
bool StoryHeistSystem::deliverScore(){ if(!plan_.escaped)return false; plan_.delivered=true; return true; }
