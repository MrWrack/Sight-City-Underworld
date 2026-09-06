#pragma once
#include "game/Settings.h"
#include "core/Math.h"

class AimAssist {
public:
    static float strength(AimAssistLevel level);
    static Vec3 blendDirection(const Vec3& current,const Vec3& target,AimAssistLevel level,float dt);
};
