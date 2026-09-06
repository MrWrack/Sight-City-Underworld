#include "CharacterCreator.h"
static int wrap(int v,int n){ v%=n; if(v<0)v+=n; return v; }
void CharacterCreator::reset(){ preview=CharacterAppearance{}; }
void CharacterCreator::cycleFace(int d){preview.face=wrap(preview.face+d,FACE_COUNT);} 
void CharacterCreator::cycleHair(int d){preview.hair=wrap(preview.hair+d,HAIR_COUNT);} 
void CharacterCreator::cycleHairColor(int d){preview.hairColor=wrap(preview.hairColor+d,HAIR_COLOR_COUNT);} 
void CharacterCreator::cycleSkinTone(int d){preview.skinTone=wrap(preview.skinTone+d,SKIN_TONE_COUNT);} 
void CharacterCreator::cycleBuild(int d){preview.build=(BodyBuild)wrap((int)preview.build+d,4);} 
void CharacterCreator::cycleTop(int d){preview.top=wrap(preview.top+d,TOP_COUNT);} 
void CharacterCreator::cycleBottoms(int d){preview.bottoms=wrap(preview.bottoms+d,BOTTOM_COUNT);} 
void CharacterCreator::cycleShoes(int d){preview.shoes=wrap(preview.shoes+d,SHOE_COUNT);} 
void CharacterCreator::cycleAccessory(int d){preview.accessory=wrap(preview.accessory+d,ACCESSORY_COUNT);} 
bool CharacterCreator::commit(CharacterProfile& profile,const std::string& playerName) const{return profile.setOnlineAppearance(playerName,preview);} 
