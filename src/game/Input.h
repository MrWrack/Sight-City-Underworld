#pragma once
#include <array>
#include <cstddef>

enum class InputAction : std::size_t {
    Jump = 0,
    EnterExitVehicle,
    Sprint,
    Stealth,
    Aim,
    Fire,
    WeaponWheel,
    Phone,
    VehicleAccelerate,
    VehicleBrake,
    VehicleIndicatorLeft,
    VehicleIndicatorRight,
    VehicleHazards,
    VehicleLightsCycle,
    CameraCycle,
    COUNT
};

struct ActionState {
    std::array<bool, static_cast<std::size_t>(InputAction::COUNT)> held{};
    std::array<bool, static_cast<std::size_t>(InputAction::COUNT)> pressed{};
    bool isHeld(InputAction a) const { return held[static_cast<std::size_t>(a)]; }
    bool isPressed(InputAction a) const { return pressed[static_cast<std::size_t>(a)]; }
    void set(InputAction a, bool down, bool justPressed=false) {
        held[static_cast<std::size_t>(a)] = down;
        pressed[static_cast<std::size_t>(a)] = justPressed;
    }
};

struct InputState {
    float moveX{0}, moveY{0};
    float lookX{0}, lookY{0};
    float throttle{0}, brake{0}, steer{0};
    ActionState actions{};

    bool jump() const { return actions.isPressed(InputAction::Jump); }
    bool enterExitVehicle() const { return actions.isPressed(InputAction::EnterExitVehicle); }
    bool sprint() const { return actions.isHeld(InputAction::Sprint); }
    bool stealth() const { return actions.isHeld(InputAction::Stealth); }
    bool aim() const { return actions.isHeld(InputAction::Aim); }
    bool fire() const { return actions.isPressed(InputAction::Fire); }
    bool weaponWheel() const { return actions.isHeld(InputAction::WeaponWheel); }
    bool phone() const { return actions.isPressed(InputAction::Phone); }
    bool accelerate() const { return actions.isHeld(InputAction::VehicleAccelerate); }
    bool braking() const { return actions.isHeld(InputAction::VehicleBrake); }
    bool indicatorLeft() const { return actions.isPressed(InputAction::VehicleIndicatorLeft); }
    bool indicatorRight() const { return actions.isPressed(InputAction::VehicleIndicatorRight); }
    bool hazards() const { return actions.isPressed(InputAction::VehicleHazards); }
    bool lightsCycle() const { return actions.isPressed(InputAction::VehicleLightsCycle); }
    bool cameraCycle() const { return actions.isPressed(InputAction::CameraCycle); }
};
