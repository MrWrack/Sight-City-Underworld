#pragma once
#include "core/Math.h"
#include <vector>
#include <cstdint>
#include <string>
enum class PropertyType:uint8_t { Apartment, House, Villa, Penthouse, BeachHouse, Mansion, StandaloneGarage };
struct Property { uint32_t id; PropertyType type; Vec3 position; int price; bool owned; uint8_t garageSlots; bool interior; bool mainResidence=false; };
class PropertySystem {
public:
 PropertySystem(); std::vector<Property> properties;
 bool buy(uint32_t id,int& money); bool setMainResidence(uint32_t id); int ownedResidentialCount()const; int ownedStandaloneGarageCount()const;
 bool upgradeGarage(uint32_t id,uint8_t slots,int& money,int price);
 bool save(const std::string& path) const; bool load(const std::string& path);
private: bool isResidential(PropertyType t)const;
};
