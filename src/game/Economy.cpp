#include "game/Economy.h"
#include <algorithm>
EconomySystem::EconomySystem(){
 businesses={
  {1,BusinessType::Restaurant,{-900,0,500},180000,25000,9000,5000,0,0,BusinessStatus::ForSale,true},
  {2,BusinessType::Garage,{1100,0,700},320000,40000,15000,8500,0,0,BusinessStatus::ForSale,true},
  {3,BusinessType::Nightclub,{300,0,-600},750000,80000,34000,19000,0,0,BusinessStatus::ForSale,true},
  {4,BusinessType::Warehouse,{4200,0,-2300},500000,55000,21000,12000,0,0,BusinessStatus::ForSale,false},
  {5,BusinessType::GasStation,{-5100,0,3400},260000,30000,12000,6500,0,0,BusinessStatus::ForSale,false}
 };
 banks={{100,{0,0,250},true,true,0},{101,{6500,0,4200},true,false,1},{102,{-7200,0,-2800},true,false,2}};
 regions={{0,100.f,0},{1,100.f,0},{2,100.f,0}};
}
Business* EconomySystem::findBusinessMutable(uint32_t id){for(auto& b:businesses)if(b.id==id)return &b;return nullptr;}
const Business* EconomySystem::findBusiness(uint32_t id)const{for(auto& b:businesses)if(b.id==id)return &b;return nullptr;}
bool EconomySystem::buyBusiness(uint32_t id,int& p){auto*b=findBusinessMutable(id);if(!b||b->status!=BusinessStatus::ForSale||p<b->price)return false;p-=b->price;b->status=BusinessStatus::Owned;return true;}
bool EconomySystem::depositToBusiness(uint32_t id,int a,int& p){auto*b=findBusinessMutable(id);if(!b||a<=0||p<a||b->status==BusinessStatus::Bankrupt)return false;p-=a;b->account+=a;return true;}
bool EconomySystem::withdrawFromBusiness(uint32_t id,int a,int& p){auto*b=findBusinessMutable(id);if(!b||a<=0||b->account<a||b->status==BusinessStatus::Bankrupt)return false;b->account-=a;p+=a;return true;}
bool EconomySystem::takeBusinessLoan(uint32_t id,int a){auto*b=findBusinessMutable(id);if(!b||a<=0||b->status==BusinessStatus::Bankrupt)return false;b->account+=a;b->debt+=a;b->loanPayment+=std::max(100,a/20);return true;}
void EconomySystem::processDay(){for(auto&b:businesses){if(b.status!=BusinessStatus::Owned&&b.status!=BusinessStatus::Warning)continue;b.account+=b.revenuePerDay;int due=b.costsPerDay+b.loanPayment;if(b.account>=due){b.account-=due;b.status=BusinessStatus::Owned;if(b.debt>0){int pay=std::min(b.debt,b.loanPayment);b.debt-=pay;}}else{b.debt+=due-b.account;b.account=0;b.status=b.debt>b.price/3?BusinessStatus::Bankrupt:BusinessStatus::Warning;}}}
void EconomySystem::applyBankEvent(uint32_t id,float s){for(auto& bank:banks)if(bank.id==id){bank.open=false;for(auto&r:regions)if(r.regionId==bank.regionId){r.health=std::max(35.f,r.health-s*25.f);r.disruptionDays=3+(int)(s*5);}}}
void EconomySystem::update(float dt){recoveryAccumulator+=dt;if(recoveryAccumulator<60.f)return;recoveryAccumulator=0;for(auto&r:regions){if(r.disruptionDays>0)--r.disruptionDays;else r.health=std::min(100.f,r.health+1.f);}for(auto&b:banks)if(!b.open){for(auto&r:regions)if(r.regionId==b.regionId&&r.disruptionDays==0)b.open=true;}}
