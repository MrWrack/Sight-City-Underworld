#include "game/TimeOfDaySystem.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

TimeOfDaySystem::TimeOfDaySystem(int h,int m){ setTime(h,m); }
void TimeOfDaySystem::setTime(int h,int m){ h=(h%24+24)%24; m=std::max(0,std::min(59,m)); gameMinutes_=float(h*60+m); }
void TimeOfDaySystem::update(float dt){
    if(dt<=0.f) return;
    const float gameMinutesPerRealSecond=1440.f/RealSecondsPerGameDay;
    gameMinutes_ += dt*gameMinutesPerRealSecond;
    while(gameMinutes_>=1440.f){ gameMinutes_-=1440.f; ++day_; }
}
int TimeOfDaySystem::hour() const { return int(gameMinutes_/60.f)%24; }
int TimeOfDaySystem::minute() const { return int(gameMinutes_)%60; }
DayPhase TimeOfDaySystem::phase() const {
    int h=hour(); if(h>=5&&h<7)return DayPhase::Dawn; if(h>=7&&h<10)return DayPhase::Morning;
    if(h>=10&&h<17)return DayPhase::Day; if(h>=17&&h<20)return DayPhase::Evening;
    if(h>=20&&h<22)return DayPhase::Dusk; return DayPhase::Night;
}
TimeLighting TimeOfDaySystem::lighting() const {
    float h=gameMinutes_/60.f;
    float sun=std::sin((h-6.f)*3.14159265f/12.f);
    sun=std::max(-0.2f,std::min(1.f,sun));
    float day=std::max(0.08f,sun);
    TimeLighting l; l.sunHeight=sun; l.daylight=day; l.ambient=0.18f+0.82f*day; l.skyBrightness=0.12f+0.88f*day;
    l.streetLights=(h>=19.5f||h<6.5f); l.vehicleLightsRecommended=(h>=19.f||h<7.f); return l;
}
float TimeOfDaySystem::trafficMultiplier() const { int h=hour(); if((h>=7&&h<10)||(h>=16&&h<19))return 1.25f; if(h>=1&&h<5)return 0.35f; if(h>=22||h<7)return 0.60f; return 1.f; }
float TimeOfDaySystem::pedestrianMultiplier() const { int h=hour(); if(h>=1&&h<5)return 0.25f; if(h>=18&&h<23)return 1.10f; return 1.f; }
bool TimeOfDaySystem::nightclubActive() const { int h=hour(); return h>=20||h<4; }
bool TimeOfDaySystem::standardShopOpen() const { int h=hour(); return h>=8&&h<22; }
std::string TimeOfDaySystem::clockText() const { char b[16]; std::snprintf(b,sizeof(b),"%02d:%02d",hour(),minute()); return b; }
std::string TimeOfDaySystem::phaseName() const { switch(phase()){case DayPhase::Dawn:return "Dawn";case DayPhase::Morning:return "Morning";case DayPhase::Day:return "Day";case DayPhase::Evening:return "Evening";case DayPhase::Dusk:return "Dusk";default:return "Night";} }
