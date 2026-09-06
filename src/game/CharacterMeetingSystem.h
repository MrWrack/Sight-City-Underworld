#pragma once
#include "core/Math.h"
#include <string>
#include <vector>
struct CharacterMeeting { std::string character; Vec3 position; int startHour{0}; int endHour{24}; bool storyRequired{true}; };
struct MeetingMarker { std::string label; Vec3 position; bool visible{false}; bool pulsing{false}; std::string timeText; };
class CharacterMeetingSystem {
public:
    CharacterMeetingSystem();
    MeetingMarker markerFor(const std::string& character,int hour) const;
private: std::vector<CharacterMeeting> meetings_;
};
