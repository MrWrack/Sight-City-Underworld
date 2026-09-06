#include "game/WeatherSystem.h"
#include <algorithm>
#include <cmath>

static uint32_t mix32(uint32_t x){ x^=x>>16; x*=0x7feb352du; x^=x>>15; x*=0x846ca68bu; return x^(x>>16); }
float WeatherSystem::clamp01(float v){ return std::max(0.f,std::min(1.f,v)); }
WeatherSystem::WeatherSystem(){ setSeason(Season::Summer); setWeather(WeatherType::Clear); }
void WeatherSystem::setSeason(Season s){ season_=s; state_.season=s; }
void WeatherSystem::setWeather(WeatherType w){ weather_=w; state_.type=w; }
void WeatherSystem::setDaySeed(int day,uint32_t seed){
    uint32_t r=mix32(seed ^ uint32_t(day*2654435761u) ^ uint32_t(season_)); int p=int(r%100u);
    switch(season_){
      case Season::Winter: weather_=(p<18?WeatherType::HeavySnow:p<42?WeatherType::LightSnow:p<52?WeatherType::Snowstorm:p<68?WeatherType::Overcast:p<78?WeatherType::Fog:WeatherType::Clear); break;
      case Season::Spring: weather_=(p<22?WeatherType::Rain:p<31?WeatherType::HeavyRain:p<48?WeatherType::Overcast:p<60?WeatherType::Fog:p<80?WeatherType::PartlyCloudy:WeatherType::Clear); break;
      case Season::Summer: weather_=(p<10?WeatherType::Rain:p<14?WeatherType::HeavyRain:p<30?WeatherType::PartlyCloudy:p<36?WeatherType::Overcast:WeatherType::Clear); break;
      case Season::Autumn: weather_=(p<24?WeatherType::Rain:p<32?WeatherType::HeavyRain:p<54?WeatherType::Overcast:p<68?WeatherType::Fog:p<84?WeatherType::PartlyCloudy:WeatherType::Clear); break;
    } state_.type=weather_;
}
void WeatherSystem::update(float dt,const TimeOfDaySystem& time,float altitude){
    state_.season=season_; state_.type=weather_;
    float base=20.f; if(season_==Season::Winter)base=1.f; else if(season_==Season::Spring)base=11.f; else if(season_==Season::Autumn)base=9.f;
    float h=time.gameMinutes()/60.f; float dayHeat=std::max(0.f,std::sin((h-6.f)*3.14159265f/12.f))*6.f; state_.temperatureC=base+dayHeat-altitude*0.0065f;
    state_.cloudCover=0.05f; state_.precipitation=0.f; state_.wind=0.15f; state_.visibility=1.f;
    switch(weather_){
      case WeatherType::PartlyCloudy: state_.cloudCover=.45f; break; case WeatherType::Overcast: state_.cloudCover=.85f; break;
      case WeatherType::Rain: state_.cloudCover=.9f; state_.precipitation=.55f; state_.wind=.35f; break;
      case WeatherType::HeavyRain: state_.cloudCover=1.f; state_.precipitation=1.f; state_.wind=.65f; state_.visibility=.68f; break;
      case WeatherType::Fog: state_.cloudCover=.65f; state_.visibility=.35f; break;
      case WeatherType::LightSnow: state_.cloudCover=.8f; state_.precipitation=.45f; state_.visibility=.78f; break;
      case WeatherType::HeavySnow: state_.cloudCover=.95f; state_.precipitation=.85f; state_.wind=.5f; state_.visibility=.52f; break;
      case WeatherType::Snowstorm: state_.cloudCover=1.f; state_.precipitation=1.f; state_.wind=1.f; state_.visibility=.22f; break; default: break;
    }
    bool snow=(weather_==WeatherType::LightSnow||weather_==WeatherType::HeavySnow||weather_==WeatherType::Snowstorm) && state_.temperatureC<=2.5f;
    bool rain=(weather_==WeatherType::Rain||weather_==WeatherType::HeavyRain);
    targetWetness_=rain?state_.precipitation:0.f; targetSnow_=snow?std::min(1.f,state_.snowCover+state_.precipitation*.15f):0.f;
    float wetRate=rain?.12f:.025f, snowRate=snow?.045f:(state_.temperatureC>2.f?.018f:.002f);
    state_.wetness += (targetWetness_-state_.wetness)*clamp01(dt*wetRate);
    if(snow) state_.snowCover=clamp01(state_.snowCover+dt*snowRate); else if(state_.temperatureC>0.f) state_.snowCover=clamp01(state_.snowCover-dt*snowRate);
    float grip=1.f-state_.wetness*.16f-state_.snowCover*.38f; state_.roadGrip=std::max(.48f,grip);
    TimeLighting l=time.lighting(); float night=1.f-clamp01(l.skyBrightness);
    state_.skyR=.40f*l.skyBrightness+.025f*night; state_.skyG=.62f*l.skyBrightness+.07f*night; state_.skyB=.92f*l.skyBrightness+.20f*night;
    float cloudLight=.82f*l.skyBrightness+.10f*night; state_.cloudR=cloudLight*.78f; state_.cloudG=cloudLight*.84f; state_.cloudB=std::min(1.f,cloudLight*1.02f);
}
std::string WeatherSystem::seasonName() const { switch(season_){case Season::Winter:return "Winter";case Season::Spring:return "Spring";case Season::Summer:return "Summer";default:return "Autumn";} }
std::string WeatherSystem::weatherName() const { switch(weather_){case WeatherType::Clear:return "Clear";case WeatherType::PartlyCloudy:return "Partly Cloudy";case WeatherType::Overcast:return "Overcast";case WeatherType::Rain:return "Rain";case WeatherType::HeavyRain:return "Heavy Rain";case WeatherType::Fog:return "Fog";case WeatherType::LightSnow:return "Light Snow";case WeatherType::HeavySnow:return "Heavy Snow";default:return "Snowstorm";} }
