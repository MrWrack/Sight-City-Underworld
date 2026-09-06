#pragma once
#include <string>
#include <vector>
#include "GameMode.h"
#include "PhoneSystem.h"
#include "VehicleCustomization.h"

enum class GarageKind { Residential, Standalone, Business };
struct GarageRecord { std::string id; GarageKind kind; std::string ownerId; int slots=1; bool owned=false; };
struct VehicleRecord {
 std::string id,name,garageId,businessType;
 int purchasePrice=0; float condition=1.0f; float permanentWear=0.0f;
 bool heistCompatible=false,businessVehicle=false,missionLocked=false;
 bool active=false, getawayLocked=false; VehicleCustomization custom;
};
class GarageSystem {
public:
 explicit GarageSystem(GameMode mode=GameMode::Story):mode_(mode){}
 bool addGarage(const GarageRecord& g);
 bool addVehicle(const VehicleRecord& v);
 bool moveVehicle(const std::string& vehicleId,const std::string& garageId);
 bool setMissionLocked(const std::string& vehicleId,bool locked);
 bool setActiveVehicle(const std::string& vehicleId);
 bool setGetawayVehicle(const std::string& vehicleId,bool locked);
 bool repaint(const std::string& vehicleId,const std::string& primary,const std::string& secondary,const std::string& wheel,PaintFinish finish,int& bank,int& cost);
 bool restoreOriginalPaint(const std::string& vehicleId,int& bank,int& cost);
 bool setTireSmokeColor(const std::string& vehicleId,const std::string& color,int& bank,int& cost);
 bool upgrade(const std::string& vehicleId,const std::string& part,int level,int& bank,int& cost);
 bool damageVehicle(const std::string& vehicleId,float damage);
 bool repairVehicle(const std::string& vehicleId,int& bank,int& repairCost);
 int saleValue(const VehicleRecord& v) const;
 bool sellVehicle(const std::string& vehicleId,int& bank,int& credited);
 bool transferBusinessVehicles(const std::string& businessType,const std::string& newGarageId);
 int usedSlots(const std::string& garageId)const;
 int capacity(const std::string& garageId)const;
 std::vector<OwnedVehicle> phoneVehicles()const;
 const std::vector<GarageRecord>& garages()const{return garages_;}
 const std::vector<VehicleRecord>& vehicles()const{return vehicles_;}
private:
 GameMode mode_; std::vector<GarageRecord> garages_; std::vector<VehicleRecord> vehicles_;
};
