#pragma once
#include <string>
#include <cstdint>

enum class DayPhase : uint8_t { Dawn, Morning, Day, Evening, Dusk, Night };

struct TimeLighting {
    float daylight{1.f};
    float ambient{1.f};
    float sunHeight{1.f};
    float skyBrightness{1.f};
    bool streetLights{false};
    bool vehicleLightsRecommended{false};
};

class TimeOfDaySystem {
public:
    static constexpr float RealSecondsPerGameDay = 48.f * 60.f;
    TimeOfDaySystem(int hour=8, int minute=0);
    void update(float realDtSeconds);
    void setTime(int hour, int minute);
    int hour() const;
    int minute() const;
    int day() const { return day_; }
    float gameMinutes() const { return gameMinutes_; }
    DayPhase phase() const;
    TimeLighting lighting() const;
    float trafficMultiplier() const;
    float pedestrianMultiplier() const;
    bool nightclubActive() const;
    bool standardShopOpen() const;
    std::string clockText() const;
    std::string phaseName() const;
private:
    float gameMinutes_{480.f};
    int day_{1};
};
