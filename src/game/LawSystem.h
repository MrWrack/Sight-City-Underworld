#pragma once
#include "core/Math.h"
#include "game/WantedSystem.h"
#include <cstdint>

enum class TrafficOffense:uint8_t { None, Speeding, RedLight, WrongWay, RecklessDriving, HitAndRun, EvadingStop };
enum class PoliceState:uint8_t { Patrol, StopRequested, PulledOver, Citation, Pursuit, Arrested, Ambulance, Hospital, Jail };
struct Citation { TrafficOffense offense{TrafficOffense::None}; int fine{0}; };
class LawSystem {
public:
    PoliceState state{PoliceState::Patrol}; Citation lastCitation{}; float stateTimer{0}; bool policeWitness{false};
    int jailSeconds{0}; int totalFines{0};
    void report(TrafficOffense offense,bool witnessed,WantedSystem& wanted);
    void obeyStop(WantedSystem& wanted);
    void refuseStop(WantedSystem& wanted);
    void resolveCrash(float injurySeverity,WantedSystem& wanted);
    void update(float dt,WantedSystem& wanted);
    static int fineFor(TrafficOffense offense);
};
