#pragma once
#include <cstddef>
#include <string>

struct VitaStoryBudget {
    int targetFps{30};
    int streamedCellRadius{2};
    int maxActivePeds{24};
    int maxActiveTraffic{18};
    int maxWildlife{12};
    int maxDynamicIncidents{4};
    std::size_t coverAndUiBudgetBytes{16u * 1024u * 1024u};
};

struct StoryCheckpoint {
    std::string missionId;
    int objectiveIndex{0};
    int wantedLevel{0};
    long long money{0};
    int xp{0};
    bool valid{false};
};

class VitaStoryPolish {
public:
    const VitaStoryBudget& budget() const { return budget_; }
    void captureCheckpoint(const std::string& missionId, int objectiveIndex,
                           int wantedLevel, long long money, int xp);
    bool canRetry() const { return checkpoint_.valid; }
    const StoryCheckpoint& checkpoint() const { return checkpoint_; }
    bool retryFromCheckpoint(StoryCheckpoint& restored) const;

    bool validateSaveState(bool storyComplete, bool vosslerGone,
                           bool postStoryFreeRoam) const;
    bool onlineAllowed() const { return false; }
    bool hardwareTestRequiredBeforeOnline() const { return true; }
    bool vitaFrameBudgetExceeded(float frameMs) const { return frameMs > 33.3334f; }
private:
    VitaStoryBudget budget_{};
    StoryCheckpoint checkpoint_{};
};
