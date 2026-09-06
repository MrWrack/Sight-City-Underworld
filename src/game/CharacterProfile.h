#pragma once
#include "GameMode.h"
#include <string>
#include <cstdint>

// Story keeps a fixed identity. Online can freely customize the full appearance.
enum class BodyBuild : uint8_t { Slim=0, Average=1, Athletic=2, Heavy=3 };
struct CharacterAppearance {
    int face=0;
    int hair=0;
    int hairColor=0;
    int skinTone=0;
    BodyBuild build=BodyBuild::Average;
    int top=0;
    int bottoms=0;
    int shoes=0;
    int accessory=0;
};

struct CharacterStats {
    int strength=0;
    int stamina=0;
    int running=0;
    int swimming=0;
    int cycling=0;
    int driving=0;
    int flying=0;
    int stealth=0;
};

class CharacterProfile {
public:
    static constexpr int SAVE_VERSION=2;
    GameMode mode=GameMode::Story;
    std::string name="Dash";
    CharacterAppearance appearance{};
    CharacterStats stats{};
    int money=0;
    int bankMoney=0;
    int xp=0;
    int level=1;
    bool customized=false;

    // Full creator is Online-only.
    bool setOnlineAppearance(const std::string& playerName,const CharacterAppearance& value);
    // Story can change clothes/accessories without changing the fixed character identity.
    bool setStoryOutfit(int top,int bottoms,int shoes,int accessory);
    bool addXp(int amount);
    bool addMoney(int amount);
    bool save(const std::string& path) const;
    bool load(const std::string& path);
};

CharacterProfile makeStoryCharacter();
CharacterProfile makeOnlineCharacter();
