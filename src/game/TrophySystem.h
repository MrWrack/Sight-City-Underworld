#pragma once
#include <string>
#include <vector>

enum class TrophyGrade { Bronze, Silver, Gold, Platinum };

struct Trophy {
    std::string id;
    std::string name;
    std::string description;
    TrophyGrade grade;
    bool unlocked = false;
};

class TrophySystem {
public:
    TrophySystem();
    bool unlock(const std::string& id);
    bool isUnlocked(const std::string& id) const;
    const std::vector<Trophy>& trophies() const { return trophies_; }
    int unlockedCount() const;
    int totalCount() const { return (int)trophies_.size(); }
    float completionPercent() const;
    bool save(const std::string& path) const;
    bool load(const std::string& path);
private:
    void checkPlatinum();
    std::vector<Trophy> trophies_;
};
