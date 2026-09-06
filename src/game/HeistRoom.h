#pragma once
#include <string>
#include <vector>
#include "GameMode.h"
#include "PhoneSystem.h"
struct HeistRoomState { bool installed=false; std::string propertyId; std::string selectedHeist; std::string getawayVehicle; };
class HeistRoom {
public:
 bool install(const std::string& propertyId,bool propertyOwned,int& bank,int cost=25000);
 bool selectHeist(const std::string&id){if(!state_.installed)return false;state_.selectedHeist=id;return true;}
 bool selectGetawayVehicle(const std::string&id,const std::vector<OwnedVehicle>&garage);
 const HeistRoomState& state()const{return state_;}
private: HeistRoomState state_;
};
