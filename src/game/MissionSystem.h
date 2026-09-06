#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "GameMode.h"

enum class MissionCategory : uint8_t { Story, Side, Race, Heist, Delivery, PoliceEscape, Business, VehicleChallenge, Aircraft, Stunt, CoopOnline };
enum class MissionState : uint8_t { Locked, Available, Active, Completed, Failed };
enum class ObjectiveType : uint8_t { ReachLocation, EnterVehicle, Deliver, WinRace, EscapeWanted, CompleteStunt, Interact, CoopSync };

struct MissionReward { int money=0; int xp=0; std::string unlock; };
struct MissionObjective { ObjectiveType type=ObjectiveType::ReachLocation; std::string text; int target=1; int progress=0; bool complete() const { return progress>=target; } };
struct MissionDefinition {
    std::string id, title;
    MissionCategory category=MissionCategory::Side;
    std::vector<MissionObjective> objectives;
    MissionReward reward;
    bool onlineOnly=false;
    GameMode campaign=GameMode::Story;
    std::string prerequisite; // empty = immediately available
    int chapter=1;
};
struct MissionProgress { MissionState state=MissionState::Available; size_t objectiveIndex=0; size_t checkpointObjective=0; };

class MissionSystem {
public:
    void registerMission(const MissionDefinition& m);
    void refreshLocks();
    bool start(const std::string& id, bool onlineSession);
    bool start(const std::string& id, GameMode mode);
    bool addProgress(ObjectiveType type, int amount=1);
    bool fail();
    bool restartFromCheckpoint();
    bool setCheckpoint();
    const MissionDefinition* activeMission() const;
    const MissionProgress* activeProgress() const;
    MissionState stateOf(const std::string& id) const;
    int money() const { return moneyEarned; }
    int xp() const { return xpEarned; }
    const std::vector<std::string>& unlocks() const { return unlocked; }
    bool save(const std::string& path) const;
    bool load(const std::string& path);
    const std::vector<MissionDefinition>& missions() const { return defs; }
private:
    int find(const std::string& id) const;
    void completeActive();
    bool prerequisiteComplete(const MissionDefinition& d) const;
    std::vector<MissionDefinition> defs;
    std::vector<MissionProgress> progress;
    int active=-1;
    int moneyEarned=0, xpEarned=0;
    std::vector<std::string> unlocked;
};

void registerSightCityStarterMissions(MissionSystem& ms);
void registerStoryCampaign(MissionSystem& ms);
void registerOnlineCampaign(MissionSystem& ms);
