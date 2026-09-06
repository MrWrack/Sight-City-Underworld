#include "VehicleCustomization.h"
int customizationValue(const VehicleCustomization& c){
 return c.engine*6000+c.brakes*3500+c.transmission*5000+c.suspension*3000+c.tires*2200+c.rims*2800+c.lights*1200+c.cosmetic*1800;
}
