#pragma once
#include <string>

enum class AimAssistLevel { Off=0, Low=1, Normal=2, High=3 };
enum class SpeedUnit { Kmh=0, Mph=1 };

struct GameSettings {
    float lookSensitivity{1.0f};
    float stickDeadzone{0.14f};
    bool invertCameraX{false};
    bool invertCameraY{false};
    bool vibration{true};
    AimAssistLevel aimAssist{AimAssistLevel::Normal};

    // Milestone 08 camera / vehicle accessibility settings.
    bool firstPersonEnabled{true};
    bool firstPersonOnFoot{true};
    bool firstPersonInVehicle{true};
    float firstPersonFov{75.0f};
    bool vehicleIndicators{true};
    bool vehicleLights{true};
    SpeedUnit speedUnit{SpeedUnit::Kmh};
    float displaySpeed(float metersPerSecond) const { return metersPerSecond * (speedUnit==SpeedUnit::Kmh ? 3.6f : 2.23693629f); }
    const char* speedUnitText() const { return speedUnit==SpeedUnit::Kmh ? "km/h" : "mph"; }

    void resetControls();
    bool load(const std::string& path);
    bool save(const std::string& path) const;
};
