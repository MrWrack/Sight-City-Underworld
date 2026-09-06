#pragma once
#include "core/Math.h"
#include <cstdint>
#include <vector>
#include <string>

enum class BusinessType:uint8_t { Restaurant, GasStation, Garage, Nightclub, Warehouse, TaxiCompany, Supermarket };
enum class BusinessStatus:uint8_t { ForSale, Owned, Warning, Bankrupt };
struct Business {
 uint32_t id; BusinessType type; Vec3 position; int price; int account; int revenuePerDay; int costsPerDay; int debt; int loanPayment; BusinessStatus status; bool discovered;
};
struct Bank { uint32_t id; Vec3 position; bool open; bool discovered; int regionId; };
struct RegionEconomy { int regionId; float health; int disruptionDays; };
class EconomySystem {
public:
 EconomySystem();
 bool buyBusiness(uint32_t id,int& playerBank);
 bool depositToBusiness(uint32_t id,int amount,int& playerBank);
 bool withdrawFromBusiness(uint32_t id,int amount,int& playerBank);
 bool takeBusinessLoan(uint32_t id,int amount);
 void processDay();
 void applyBankEvent(uint32_t bankId,float severity);
 void update(float dt);
 const Business* findBusiness(uint32_t id) const;
 std::vector<Business> businesses; std::vector<Bank> banks; std::vector<RegionEconomy> regions;
private:
 Business* findBusinessMutable(uint32_t id); float recoveryAccumulator{0};
};
