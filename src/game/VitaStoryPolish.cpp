#include "VitaStoryPolish.h"
void VitaStoryPolish::captureCheckpoint(const std::string& missionId, int objectiveIndex,
                                        int wantedLevel, long long money, int xp) {
    checkpoint_.missionId = missionId;
    checkpoint_.objectiveIndex = objectiveIndex;
    checkpoint_.wantedLevel = wantedLevel;
    checkpoint_.money = money;
    checkpoint_.xp = xp;
    checkpoint_.valid = !missionId.empty() && objectiveIndex >= 0;
}
bool VitaStoryPolish::retryFromCheckpoint(StoryCheckpoint& restored) const {
    if (!checkpoint_.valid) return false;
    restored = checkpoint_;
    return true;
}
bool VitaStoryPolish::validateSaveState(bool storyComplete, bool vosslerGone,
                                        bool postStoryFreeRoam) const {
    if (postStoryFreeRoam && !storyComplete) return false;
    if (storyComplete && !vosslerGone) return false;
    return true;
}
