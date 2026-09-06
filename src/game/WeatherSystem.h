#pragma once
#include "game/TimeOfDaySystem.h"
#include <cstdint>
#include <string>

enum class Season : uint8_t { Winter, Spring, Summer, Autumn };
enum class WeatherType : uint8_t { Clear, PartlyCloudy, Overcast, Rain, HeavyRain, Fog, LightSnow, HeavySnow, Snowstorm };

struct WeatherState {
    Season season{Season::Summer};
    WeatherType type{WeatherType::Clear};
    float temperatureC{20.f};
    float cloudCover{0.f};
    float precipitation{0.f};
    float wind{0.15f};
    float visibility{1.f};
    float wetness{0.f};
    float snowCover{0.f};
    float roadGrip{1.f};
    float skyR{0.45f}, skyG{0.68f}, skyB{0.95f};
    float cloudR{0.9f}, cloudG{0.9f}, cloudB{0.92f};
};

class WeatherSystem {
public:
    WeatherSystem();
    void setSeason(Season season);
    void setWeather(WeatherType type);
    void setDaySeed(int day, uint32_t worldSeed=0x53494748u);
    void update(float realDtSeconds, const TimeOfDaySystem& time, float altitudeMeters=0.f);
    Season season() const { return season_; }
    WeatherType weather() const { return weather_; }
    const WeatherState& state() const { return state_; }
    std::string seasonName() const;
    std::string weatherName() const;
private:
    Season season_{Season::Summer}; WeatherType weather_{WeatherType::Clear}; WeatherState state_{};
    float targetWetness_{0.f}, targetSnow_{0.f};
    static float clamp01(float v);
};
