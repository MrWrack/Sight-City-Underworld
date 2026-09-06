#pragma once
#include "game/NPC.h"
#include "game/CharacterAI.h"
#include "game/WantedSystem.h"
#include <string>
#include <vector>
#include <cstdint>

enum class NPCCrime:uint8_t { None, Theft, Robbery, ArmedRobbery, RecklessDriving, Speeding };
struct NPCCrimeEvent { size_t npcIndex{0}; NPCCrime crime{NPCCrime::None}; bool witnessed{false}; bool policeResponding{false}; float timer{0}; };
class WorldAI {
public:
    NPCSystem pedestrians;
    CharacterAISystem storyCharacters;
    void update(const Vec3& player,float dt,RegionType region,bool playerDanger);
    const std::vector<NPCCrimeEvent>& crimeEvents() const { return crimes; }
    int policeResponses() const;
private:
    float simTime{0};
    std::vector<NPCCrimeEvent> crimes;
};
