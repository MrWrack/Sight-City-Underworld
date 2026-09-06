#include "HeistRoom.h"
bool HeistRoom::install(const std::string&p,bool owned,int&bank,int cost){if(!owned||state_.installed||bank<cost)return false;bank-=cost;state_.installed=true;state_.propertyId=p;return true;}
bool HeistRoom::selectGetawayVehicle(const std::string&id,const std::vector<OwnedVehicle>&g){if(!state_.installed)return false;for(const auto&v:g)if(v.id==id&&v.heistCompatible&&!v.missionLocked){state_.getawayVehicle=id;return true;}return false;}
