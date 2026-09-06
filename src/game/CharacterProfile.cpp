#include "CharacterProfile.h"
#include <fstream>
#include <iomanip>
#include <algorithm>

static int clampIndex(int v,int lo,int hi){ return std::max(lo,std::min(hi,v)); }
static CharacterAppearance sanitize(CharacterAppearance a){
    a.face=clampIndex(a.face,0,15);
    a.hair=clampIndex(a.hair,0,23);
    a.hairColor=clampIndex(a.hairColor,0,15);
    a.skinTone=clampIndex(a.skinTone,0,11);
    a.top=clampIndex(a.top,0,63);
    a.bottoms=clampIndex(a.bottoms,0,47);
    a.shoes=clampIndex(a.shoes,0,31);
    a.accessory=clampIndex(a.accessory,0,31);
    int b=(int)a.build; if(b<0||b>3)a.build=BodyBuild::Average;
    return a;
}

bool CharacterProfile::setOnlineAppearance(const std::string& playerName,const CharacterAppearance& value){
    if(mode!=GameMode::Online) return false;
    name=playerName.empty()?"Online Player":playerName;
    appearance=sanitize(value);
    customized=true;
    return true;
}

bool CharacterProfile::setStoryOutfit(int top,int bottoms,int shoes,int accessory){
    if(mode!=GameMode::Story) return false;
    appearance.top=clampIndex(top,0,63);
    appearance.bottoms=clampIndex(bottoms,0,47);
    appearance.shoes=clampIndex(shoes,0,31);
    appearance.accessory=clampIndex(accessory,0,31);
    return true;
}

bool CharacterProfile::addXp(int amount){
    if(amount<=0) return false;
    xp+=amount;
    while(xp>=level*1000){ xp-=level*1000; ++level; }
    return true;
}

bool CharacterProfile::addMoney(int amount){ if(amount==0)return false; money=std::max(0,money+amount); return true; }

bool CharacterProfile::save(const std::string& path) const{
    std::ofstream f(path); if(!f)return false;
    f<<"CLPROFILE "<<SAVE_VERSION<<'\n';
    f<<(int)mode<<' '<<std::quoted(name)<<' '<<money<<' '<<bankMoney<<' '<<xp<<' '<<level<<' '<<customized<<'\n';
    f<<appearance.face<<' '<<appearance.hair<<' '<<appearance.hairColor<<' '<<appearance.skinTone<<' '<<(int)appearance.build<<' '
     <<appearance.top<<' '<<appearance.bottoms<<' '<<appearance.shoes<<' '<<appearance.accessory<<'\n';
    f<<stats.strength<<' '<<stats.stamina<<' '<<stats.running<<' '<<stats.swimming<<' '<<stats.cycling<<' '
     <<stats.driving<<' '<<stats.flying<<' '<<stats.stealth<<'\n';
    return true;
}

bool CharacterProfile::load(const std::string& path){
    std::ifstream f(path); if(!f)return false;
    std::string magic; int ver=0; f>>magic>>ver;
    if(magic!="CLPROFILE" || ver<1 || ver>SAVE_VERSION) return false;
    int m,b; f>>m>>std::quoted(name)>>money;
    if(ver>=2) f>>bankMoney; else bankMoney=0;
    f>>xp>>level>>customized;
    mode=(m==(int)GameMode::Online)?GameMode::Online:GameMode::Story;
    f>>appearance.face>>appearance.hair>>appearance.hairColor>>appearance.skinTone>>b>>appearance.top>>appearance.bottoms>>appearance.shoes>>appearance.accessory;
    appearance.build=(b>=0&&b<=3)?(BodyBuild)b:BodyBuild::Average;
    appearance=sanitize(appearance);
    if(ver>=2){
        f>>stats.strength>>stats.stamina>>stats.running>>stats.swimming>>stats.cycling>>stats.driving>>stats.flying>>stats.stealth;
    }
    return (bool)f;
}

CharacterProfile makeStoryCharacter(){
    CharacterProfile p; p.mode=GameMode::Story; p.name="Dash"; p.customized=false; p.money=2500; p.bankMoney=5000;
    p.appearance.face=2; p.appearance.hair=3; p.appearance.hairColor=2; p.appearance.skinTone=3; p.appearance.build=BodyBuild::Athletic;
    return p;
}
CharacterProfile makeOnlineCharacter(){ CharacterProfile p; p.mode=GameMode::Online;p.name="Online Player";p.money=1000;p.bankMoney=2500;return p; }
