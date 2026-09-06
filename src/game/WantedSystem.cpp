#include "game/WantedSystem.h"
#include "core/Math.h"
static void syncLevel(WantedSystem&w){if(w.heat<=0){w.level=0;return;} w.level=1+(int)(w.heat/15.0f); if(w.level>7)w.level=7;}
void WantedSystem::addHeat(float a){heat=clampf(heat+a,0.0f,100.0f);syncLevel(*this);} 
void WantedSystem::reduceHeat(float a){heat=clampf(heat-a,0.0f,100.0f);syncLevel(*this);} 
void WantedSystem::clear(){heat=0;level=0;}
void WantedSystem::update(float dt,bool seen){if(!seen){heat=clampf(heat-dt*0.8f,0.0f,100.0f);syncLevel(*this);}}
