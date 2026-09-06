#include "CharacterMeetingSystem.h"
#include <cstdio>
CharacterMeetingSystem::CharacterMeetingSystem(){
 meetings_.push_back({"Rhys",{-450,0,1000},20,23,true});
 meetings_.push_back({"Mayja",{520,0,-760},18,23,true});
 meetings_.push_back({"Kane",{1300,0,400},21,24,true});
}
MeetingMarker CharacterMeetingSystem::markerFor(const std::string&name,int hour)const{
 for(const auto&m:meetings_) if(m.character==name){char b[32];std::snprintf(b,sizeof(b),"%02d:00-%02d:00",m.startHour,m.endHour);bool active=hour>=m.startHour&&hour<m.endHour;return {m.character,m.position,true,active,b};}
 return {};
}
