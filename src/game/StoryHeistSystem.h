#pragma once
#include <string>

enum class EscapeRoute : unsigned char { None, Highway, Tunnels, Backroads };

struct StoryHeistPlan {
    bool mayjaMet=false;
    std::string getawayVehicle;
    bool equipmentReady=false;
    EscapeRoute escapeRoute=EscapeRoute::None;
    bool scoreTaken=false;
    bool escaped=false;
    bool delivered=false;
    bool ready() const { return mayjaMet && !getawayVehicle.empty() && equipmentReady && escapeRoute!=EscapeRoute::None; }
};

class StoryHeistSystem {
public:
    void reset();
    bool meetMayja();
    bool selectGetawayVehicle(const std::string& vehicleId,bool owned,bool heistCompatible,bool missionLocked);
    bool collectEquipment();
    bool chooseEscapeRoute(EscapeRoute route);
    bool takeScore();
    bool markEscaped();
    bool deliverScore();
    const StoryHeistPlan& plan() const { return plan_; }
private:
    StoryHeistPlan plan_{};
};
