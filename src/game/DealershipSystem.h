#pragma once
#include <string>
#include <vector>
#include "GarageSystem.h"

enum class VehicleShopType { Dealer, Motorcycle, Marine, Aircraft, Used };
struct DealerStockItem {
 std::string stockId, model, brand;
 VehicleShopType shopType=VehicleShopType::Dealer;
 int price=0;
 float condition=1.0f;
 bool used=false, heistCompatible=false;
};
struct TestDriveState { bool active=false; std::string stockId; float secondsLeft=0.0f; };
class DealershipSystem {
public:
 void addStock(const DealerStockItem& item);
 const std::vector<DealerStockItem>& stock() const { return stock_; }
 std::vector<DealerStockItem> browse(VehicleShopType type) const;
 bool startTestDrive(const std::string& stockId,float seconds=120.0f);
 void update(float dt);
 const TestDriveState& testDrive() const { return testDrive_; }
 bool buy(const std::string& stockId,const std::string& garageId,GarageSystem& garage,int& bank,std::string& purchasedVehicleId);
 bool reserveOnline(const std::string& stockId,const std::string& garageId,GarageSystem& garage,int& bank,std::string& purchasedVehicleId);
private:
 const DealerStockItem* find(const std::string& id) const;
 std::vector<DealerStockItem> stock_;
 TestDriveState testDrive_;
 unsigned serial_=1;
};
