#include "game/AimAssist.h"

float AimAssist::strength(AimAssistLevel level) {
    switch(level){
        case AimAssistLevel::Low: return 2.0f;
        case AimAssistLevel::Normal: return 4.0f;
        case AimAssistLevel::High: return 7.0f;
        default: return 0.0f;
    }
}

Vec3 AimAssist::blendDirection(const Vec3& current,const Vec3& target,AimAssistLevel level,float dt) {
    float t=strength(level)*dt; if(t>1.0f)t=1.0f;
    Vec3 mixed=current*(1.0f-t)+target*t;
    return length(mixed)>0.0001f?normalize(mixed):current;
}
