#include "TrophySystem.h"
#include <fstream>

TrophySystem::TrophySystem() {
    trophies_ = {
        {"WELCOME_BACK","Welcome Back","Complete Back in Sight City.",TrophyGrade::Bronze},
        {"OLD_TIMES","Like Old Times","Complete Like Old Times.",TrophyGrade::Bronze},
        {"EASY_MONEY","Easy Money","Complete Easy Money.",TrophyGrade::Bronze},
        {"LONG_WAY_HOME","The Long Way Home","Complete The Long Way Home.",TrophyGrade::Bronze},
        {"FIRST_SCORE","Our First Score","Complete your first heist.",TrophyGrade::Silver},
        {"STREET_RACER","Street Racer","Win your first car race.",TrophyGrade::Bronze},
        {"PEDAL_POWER","Pedal Power","Win your first bicycle event.",TrophyGrade::Bronze},
        {"WANTED","Wanted","Escape a police pursuit.",TrophyGrade::Bronze},
        {"EXPLORER","Sightseer","Discover every major Sight City region.",TrophyGrade::Silver},
        {"PROPERTY_OWNER","Property Owner","Buy your first property.",TrophyGrade::Bronze},
        {"BUSINESS_OWNER","Underworld Business","Own your first business.",TrophyGrade::Bronze},
        {"FULL_GARAGE","Full Garage","Own 10 vehicles.",TrophyGrade::Silver},
        {"HIGH_FLYER","High Flyer","Successfully take off and land an aircraft.",TrophyGrade::Bronze},
        {"CENTRAL_RESERVE","The Big Score","Complete the Central Reserve heist.",TrophyGrade::Gold},
        {"TAKE_SIGHT_CITY","Take Sight City","Complete the final Story mission.",TrophyGrade::Gold},
        {"STORY_COMPLETE","Underworld","Complete the Story.",TrophyGrade::Gold},
        {"COMPLETE_IT_ALL","Complete It All","Unlock every other trophy in Sight City: Underworld.",TrophyGrade::Platinum}
    };
}

bool TrophySystem::unlock(const std::string& id) {
    if (id == "COMPLETE_IT_ALL") return false;
    for (auto& t : trophies_) if (t.id == id) {
        bool changed = !t.unlocked; t.unlocked = true; checkPlatinum(); return changed;
    }
    return false;
}

bool TrophySystem::isUnlocked(const std::string& id) const {
    for (const auto& t : trophies_) if (t.id == id) return t.unlocked;
    return false;
}

int TrophySystem::unlockedCount() const { int n=0; for(const auto& t:trophies_) if(t.unlocked) ++n; return n; }
float TrophySystem::completionPercent() const { return trophies_.empty()?0.0f:100.0f*unlockedCount()/trophies_.size(); }

void TrophySystem::checkPlatinum() {
    bool all = true;
    for (const auto& t : trophies_) if (t.grade != TrophyGrade::Platinum && !t.unlocked) { all=false; break; }
    if (all) for (auto& t : trophies_) if (t.grade == TrophyGrade::Platinum) t.unlocked = true;
}

bool TrophySystem::save(const std::string& path) const {
    std::ofstream f(path); if(!f) return false;
    for(const auto& t:trophies_) f << t.id << " " << (t.unlocked?1:0) << "\n";
    return true;
}
bool TrophySystem::load(const std::string& path) {
    std::ifstream f(path); if(!f) return false; std::string id; int v;
    while(f>>id>>v) if(v && id!="COMPLETE_IT_ALL") unlock(id);
    checkPlatinum(); return true;
}
