#pragma once
#include "core/Math.h"
#include <string>
#include <vector>
#include <cstdint>

enum class CharacterRole:uint8_t { Protagonist, BestFriend, Planner, Security, Antagonist, Crew };
enum class AIIntent:uint8_t { Follow, Hold, MoveTo, TakeCover, Fight, Flee, Drive, Plan };
struct StoryCharacterAI {
    std::string id;
    std::string name;
    CharacterRole role{CharacterRole::Crew};
    Vec3 position{};
    AIIntent intent{AIIntent::Hold};
    float health{100.0f};
    float loyalty{1.0f};
    bool alive{true};
    bool available{true};
};
class CharacterAISystem {
public:
    CharacterAISystem();
    void update(const Vec3& dashPosition,float dt,bool combat);
    StoryCharacterAI* find(const std::string& id);
    const std::vector<StoryCharacterAI>& characters() const { return roster; }
    int activeAllies() const;
private:
    std::vector<StoryCharacterAI> roster;
};
