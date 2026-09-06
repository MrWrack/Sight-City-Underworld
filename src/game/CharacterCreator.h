#pragma once
#include "CharacterProfile.h"

// Lightweight data-driven creator suitable for Vita menus. Rendering/UI can consume these counts.
class CharacterCreator {
public:
    static constexpr int FACE_COUNT=16;
    static constexpr int HAIR_COUNT=24;
    static constexpr int HAIR_COLOR_COUNT=16;
    static constexpr int SKIN_TONE_COUNT=12;
    static constexpr int TOP_COUNT=64;
    static constexpr int BOTTOM_COUNT=48;
    static constexpr int SHOE_COUNT=32;
    static constexpr int ACCESSORY_COUNT=32;

    CharacterAppearance preview{};
    void reset();
    void cycleFace(int delta);
    void cycleHair(int delta);
    void cycleHairColor(int delta);
    void cycleSkinTone(int delta);
    void cycleBuild(int delta);
    void cycleTop(int delta);
    void cycleBottoms(int delta);
    void cycleShoes(int delta);
    void cycleAccessory(int delta);
    bool commit(CharacterProfile& profile,const std::string& playerName) const;
};
