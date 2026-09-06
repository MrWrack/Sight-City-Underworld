#pragma once
#include <string>

enum class PaintFinish { Gloss, Metallic, Matte };
struct VehicleCustomization {
 std::string originalPrimary="#202020", originalSecondary="#202020";
 std::string primary="#202020", secondary="#202020", wheel="#A0A0A0";
 std::string tireSmoke="#FFFFFF";
 PaintFinish finish=PaintFinish::Gloss;
 int engine=0, brakes=0, transmission=0, suspension=0, tires=0, rims=0, lights=0, cosmetic=0;
};
int customizationValue(const VehicleCustomization& c);
