#pragma once
#include "game/Input.h"
#include "game/Settings.h"
#include <psp2/ctrl.h>

class VitaInput {
public:
    VitaInput();
    InputState poll(const GameSettings& settings, bool inVehicle);
    bool quitRequested() const { return quit_; }
private:
    SceCtrlData previous_{};
    bool quit_{false};
    static float axis(unsigned char v, float deadzone);
};
