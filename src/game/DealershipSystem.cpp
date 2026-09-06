#include "DealershipSystem.h"
#include <algorithm>
void DealershipSystem::addStock(const DealerStockItem& item){for(const auto&x:stock_)if(x.stockId==item.stockId)return;stock_.push_back(item);}
const DealerStockItem* DealershipSystem::find(const std::string&id)const{for(const auto&x:stock_)if(x.stockId==id)return &x;return nullptr;}
std::vector<DealerStockItem> DealershipSystem::browse(VehicleShopType type)const{std::vector<DealerStockItem> o;for(const auto&x:stock_)if(x.shopType==type)o.push_back(x);return o;}
bool DealershipSystem::startTestDrive(const std::string&id,float seconds){if(!find(id)||seconds<=0)return false;testDrive_={true,id,seconds};return true;}
void DealershipSystem::update(float dt){if(!testDrive_.active)return;testDrive_.secondsLeft-=dt;if(testDrive_.secondsLeft<=0){testDrive_.secondsLeft=0;testDrive_.active=false;}}
bool DealershipSystem::buy(const std::string&id,const std::string&garageId,GarageSystem&garage,int&bank,std::string&out){const auto*i=find(id);if(!i||bank<i->price||garage.capacity(garageId)<=garage.usedSlots(garageId))return false;VehicleRecord v;v.id="owned_"+std::to_string(serial_++);v.name=i->brand+" "+i->model;v.garageId=garageId;v.purchasePrice=i->price;v.condition=i->condition;v.heistCompatible=i->heistCompatible;if(!garage.addVehicle(v))return false;bank-=i->price;out=v.id;return true;}
bool DealershipSystem::reserveOnline(const std::string&id,const std::string&garageId,GarageSystem&garage,int&bank,std::string&out){return buy(id,garageId,garage,bank,out);}
