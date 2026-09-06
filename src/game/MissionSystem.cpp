#include "MissionSystem.h"
#include <fstream>
#include <algorithm>

int MissionSystem::find(const std::string& id) const { for(size_t i=0;i<defs.size();++i) if(defs[i].id==id) return (int)i; return -1; }
bool MissionSystem::prerequisiteComplete(const MissionDefinition& d) const { if(d.prerequisite.empty())return true; int i=find(d.prerequisite); return i>=0 && progress[i].state==MissionState::Completed; }
void MissionSystem::registerMission(const MissionDefinition& m){ if(find(m.id)>=0)return; defs.push_back(m); MissionProgress p; p.state=m.prerequisite.empty()?MissionState::Available:MissionState::Locked; progress.push_back(p); }
void MissionSystem::refreshLocks(){ for(size_t i=0;i<defs.size();++i) if(progress[i].state==MissionState::Locked && prerequisiteComplete(defs[i])) progress[i].state=MissionState::Available; }
bool MissionSystem::start(const std::string& id,bool online){ return start(id, online?GameMode::Online:GameMode::Story); }
bool MissionSystem::start(const std::string& id,GameMode mode){ refreshLocks(); int i=find(id); if(i<0||defs[i].campaign!=mode)return false; auto&s=progress[i]; if(s.state==MissionState::Completed||s.state==MissionState::Locked)return false; active=i;s.state=MissionState::Active;s.objectiveIndex=0;s.checkpointObjective=0; for(auto&o:defs[i].objectives)o.progress=0; return true; }
bool MissionSystem::addProgress(ObjectiveType t,int amount){ if(active<0)return false; auto&d=defs[active];auto&p=progress[active]; if(p.state!=MissionState::Active||p.objectiveIndex>=d.objectives.size())return false; auto&o=d.objectives[p.objectiveIndex]; if(o.type!=t)return false; o.progress=std::min(o.target,o.progress+amount); if(o.complete()){ p.objectiveIndex++; if(p.objectiveIndex>=d.objectives.size()) completeActive(); } return true; }
void MissionSystem::completeActive(){ auto&d=defs[active];auto&p=progress[active];p.state=MissionState::Completed; moneyEarned+=d.reward.money;xpEarned+=d.reward.xp;if(!d.reward.unlock.empty() && std::find(unlocked.begin(),unlocked.end(),d.reward.unlock)==unlocked.end())unlocked.push_back(d.reward.unlock);active=-1;refreshLocks(); }
bool MissionSystem::fail(){ if(active<0)return false;progress[active].state=MissionState::Failed;return true; }
bool MissionSystem::restartFromCheckpoint(){ if(active<0)return false;auto&p=progress[active];auto&d=defs[active];p.state=MissionState::Active;p.objectiveIndex=p.checkpointObjective;for(size_t i=p.checkpointObjective;i<d.objectives.size();++i)d.objectives[i].progress=0;return true; }
bool MissionSystem::setCheckpoint(){ if(active<0)return false;progress[active].checkpointObjective=progress[active].objectiveIndex;return true; }
const MissionDefinition* MissionSystem::activeMission()const{return active>=0?&defs[active]:nullptr;}
const MissionProgress* MissionSystem::activeProgress()const{return active>=0?&progress[active]:nullptr;}
MissionState MissionSystem::stateOf(const std::string& id) const{ int i=find(id); return i>=0?progress[i].state:MissionState::Locked; }
bool MissionSystem::save(const std::string& path)const{std::ofstream f(path);if(!f)return false;f<<"CLMISSION 2\n"<<moneyEarned<<' '<<xpEarned<<'\n';for(size_t i=0;i<defs.size();++i)f<<defs[i].id<<' '<<(int)progress[i].state<<' '<<progress[i].objectiveIndex<<' '<<progress[i].checkpointObjective<<'\n';return true;}
bool MissionSystem::load(const std::string& path){std::ifstream f(path);if(!f)return false;std::string magic;int ver;f>>magic>>ver;if(magic!="CLMISSION"||ver!=2)return false;f>>moneyEarned>>xpEarned;std::string id;int st;size_t oi,cp;while(f>>id>>st>>oi>>cp){int i=find(id);if(i>=0){progress[i].state=(MissionState)st;progress[i].objectiveIndex=oi;progress[i].checkpointObjective=cp;}}refreshLocks();return true;}

static MissionDefinition M(const char*id,const char*t,MissionCategory c,ObjectiveType a,ObjectiveType b,int cash,int xp,const char*u=""){MissionDefinition m;m.id=id;m.title=t;m.category=c;m.objectives={{a,"Objective 1",1,0},{b,"Objective 2",1,0}};m.reward={cash,xp,u};return m;}
static MissionDefinition CM(const char*id,const char*t,MissionCategory c,GameMode mode,ObjectiveType a,ObjectiveType b,int cash,int xp,const char*u="",const char*pre="",int chapter=1){auto m=M(id,t,c,a,b,cash,xp,u);m.campaign=mode;m.onlineOnly=(mode==GameMode::Online);m.prerequisite=pre?pre:"";m.chapter=chapter;return m;}

void registerSightCityStarterMissions(MissionSystem& s){
 s.registerMission(M("story_welcome","Welcome to Sight City",MissionCategory::Story,ObjectiveType::ReachLocation,ObjectiveType::EnterVehicle,1200,100,"Downtown Jobs"));
 auto c=M("coop_escape","Co-op Escape",MissionCategory::CoopOnline,ObjectiveType::CoopSync,ObjectiveType::EscapeWanted,4500,280,"Co-op Jobs");c.onlineOnly=true;c.campaign=GameMode::Online;s.registerMission(c);
}

void registerStoryCampaign(MissionSystem& s){
 MissionDefinition m;
 m.id="story_01_back_in_sight_city"; m.title="Back in Sight City"; m.category=MissionCategory::Story; m.campaign=GameMode::Story; m.chapter=1;
 m.objectives={
  {ObjectiveType::ReachLocation,"Meet Rhys",1,0},
  {ObjectiveType::EnterVehicle,"Get in Rhys's car",1,0},
  {ObjectiveType::ReachLocation,"Drive through Downtown",1,0},
  {ObjectiveType::ReachLocation,"Visit the West Suburbs",1,0},
  {ObjectiveType::ReachLocation,"Follow the road to the East Coast",1,0},
  {ObjectiveType::ReachLocation,"Drive through North Countryside",1,0},
  {ObjectiveType::ReachLocation,"Reach the Mount Ridge overlook",1,0},
  {ObjectiveType::ReachLocation,"Return to Sight City after dark",1,0},
  {ObjectiveType::Interact,"Finish the tour with Rhys",1,0}
 };
 m.reward={5000,500,"Mission 2 - Like Old Times"}; s.registerMission(m);
 MissionDefinition m2;
 m2.id="story_02_like_old_times"; m2.title="Like Old Times"; m2.category=MissionCategory::Story; m2.campaign=GameMode::Story; m2.prerequisite="story_01_back_in_sight_city"; m2.chapter=1;
 m2.objectives={
  {ObjectiveType::ReachLocation,"Meet Rhys at the old garage",1,0},
  {ObjectiveType::EnterVehicle,"Get in the delivery car",1,0},
  {ObjectiveType::ReachLocation,"Drive the car to the delivery point",1,0},
  {ObjectiveType::EscapeWanted,"Lose the rival gang",1,0},
  {ObjectiveType::ReachLocation,"Reach the old workshop",1,0},
  {ObjectiveType::Interact,"Finish the job with Rhys",1,0}
 };
 m2.reward={8000,650,"Mission 3 - Easy Money"}; s.registerMission(m2);
 MissionDefinition m3;
 m3.id="story_03_easy_money"; m3.title="Easy Money"; m3.category=MissionCategory::Story; m3.campaign=GameMode::Story; m3.prerequisite="story_02_like_old_times"; m3.chapter=1;
 m3.objectives={
  {ObjectiveType::ReachLocation,"Go to Harbor Docks",1,0},
  {ObjectiveType::Interact,"Locate the electronics container",1,0},
  {ObjectiveType::EnterVehicle,"Steal the container truck",1,0},
  {ObjectiveType::EscapeWanted,"Lose the rival vehicles",1,0},
  {ObjectiveType::Deliver,"Deliver the electronics",1,0}
 };
 m3.reward={12500,800,"Mission 4 - The Long Way Home"}; s.registerMission(m3);
 MissionDefinition m4;
 m4.id="story_04_long_way_home"; m4.title="The Long Way Home"; m4.category=MissionCategory::Story; m4.campaign=GameMode::Story; m4.prerequisite="story_03_easy_money"; m4.chapter=1;
 m4.objectives={
  {ObjectiveType::ReachLocation,"Escape the Industrial Zone",1,0},
  {ObjectiveType::ReachLocation,"Get onto the highway",1,0},
  {ObjectiveType::ReachLocation,"Enter the mountain tunnels",1,0},
  {ObjectiveType::EscapeWanted,"Lose the police pursuit",1,0},
  {ObjectiveType::ReachLocation,"Reach the North Countryside garage",1,0}
 };
 m4.reward={18000,1200,"Mission 5 - Our First Score"}; s.registerMission(m4);

 MissionDefinition m5;
 m5.id="story_05_our_first_score"; m5.title="Our First Score"; m5.category=MissionCategory::Heist; m5.campaign=GameMode::Story; m5.prerequisite="story_04_long_way_home"; m5.chapter=2;
 m5.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja at the Heist Room",1,0},
  {ObjectiveType::Interact,"Choose an owned heist-compatible getaway vehicle",1,0},
  {ObjectiveType::Interact,"Collect the heist equipment",1,0},
  {ObjectiveType::Interact,"Choose the escape route",1,0},
  {ObjectiveType::ReachLocation,"Reach the armored transport route",1,0},
  {ObjectiveType::Interact,"Take the valuables from the transport",1,0},
  {ObjectiveType::EnterVehicle,"Get into the getaway vehicle",1,0},
  {ObjectiveType::EscapeWanted,"Lose the police and reach the safe route",1,0},
  {ObjectiveType::Deliver,"Deliver the score to the safehouse",1,0}
 };
 m5.reward={75000,3000,"Heists Unlocked"}; s.registerMission(m5);

 MissionDefinition m6;
 m6.id="story_06_somethings_off"; m6.title="Something\'s Off"; m6.category=MissionCategory::Story; m6.campaign=GameMode::Story; m6.prerequisite="story_05_our_first_score"; m6.chapter=2;
 m6.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja after the first score",1,0},
  {ObjectiveType::Interact,"Review the next job without Rhys",1,0},
  {ObjectiveType::ReachLocation,"Find Rhys near the rival district",1,0},
  {ObjectiveType::Interact,"Observe Rhys from a safe distance",1,0},
  {ObjectiveType::ReachLocation,"Follow Rhys without being detected (optional route)",1,0},
  {ObjectiveType::Interact,"Return to Mayja with what you saw",1,0}
 };
 m6.reward={30000,2500,"Mission 7 - Wrong Place, Wrong Time"}; s.registerMission(m6);

 MissionDefinition m7;
 m7.id="story_07_wrong_place_wrong_time"; m7.title="Wrong Place, Wrong Time"; m7.category=MissionCategory::Story; m7.campaign=GameMode::Story; m7.prerequisite="story_06_somethings_off"; m7.chapter=2;
 m7.objectives={
  {ObjectiveType::ReachLocation,"Meet Rhys for the Industrial Zone job",1,0},
  {ObjectiveType::ReachLocation,"Enter the industrial job site",1,0},
  {ObjectiveType::Interact,"Investigate the suspicious loading yard",1,0},
  {ObjectiveType::Interact,"Survive the rival gang ambush",1,0},
  {ObjectiveType::Interact,"Regroup with Rhys",1,0},
  {ObjectiveType::EscapeWanted,"Escape the Industrial Zone",1,0},
  {ObjectiveType::Interact,"Debrief Mayja at the safehouse",1,0}
 };
 m7.reward={35000,3000,"Mission 8 - Behind My Back"}; s.registerMission(m7);

 MissionDefinition m8;
 m8.id="story_08_behind_my_back"; m8.title="Behind My Back"; m8.category=MissionCategory::Story; m8.campaign=GameMode::Story; m8.prerequisite="story_07_wrong_place_wrong_time"; m8.chapter=2;
 m8.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja at the safehouse",1,0},
  {ObjectiveType::ReachLocation,"Follow Rhys without being detected",1,0},
  {ObjectiveType::Interact,"Photograph Rhys meeting the rival gang leader",1,0},
  {ObjectiveType::ReachLocation,"Leave the area without alerting Rhys",1,0},
  {ObjectiveType::Interact,"Show the evidence to Mayja",1,0}
 };
 m8.reward={38000,3500,"Mission 9 - Betrayed"}; s.registerMission(m8);

 MissionDefinition m9;
 m9.id="story_09_betrayed"; m9.title="Betrayed"; m9.category=MissionCategory::Story; m9.campaign=GameMode::Story; m9.prerequisite="story_08_behind_my_back"; m9.chapter=2;
 m9.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja and prepare the fake heist",1,0},
  {ObjectiveType::Interact,"Plant false heist information",1,0},
  {ObjectiveType::Interact,"Watch the rivals move on the exact location",1,0},
  {ObjectiveType::ReachLocation,"Confront Rhys",1,0},
  {ObjectiveType::Interact,"Hear Rhys out",1,0},
  {ObjectiveType::Interact,"Learn who is really behind the leaks",1,0}
 };
 m9.reward={40000,4000,"Mission 10 - Follow the Money"}; s.registerMission(m9);

 MissionDefinition m10; m10.id="story_10_follow_the_money"; m10.title="Follow the Money"; m10.category=MissionCategory::Story; m10.campaign=GameMode::Story; m10.prerequisite="story_09_betrayed"; m10.chapter=3;
 m10.objectives={{ObjectiveType::ReachLocation,"Trace the payment through the Industrial Zone",1,0},{ObjectiveType::Interact,"Recover the shell-company payment record",1,0},{ObjectiveType::ReachLocation,"Follow the courier into Downtown",1,0},{ObjectiveType::ReachLocation,"Track the transfer to the Airport",1,0},{ObjectiveType::ReachLocation,"Follow the final money trail to the West Coast",1,0},{ObjectiveType::Interact,"Identify Vossler Holdings and Project Sight",1,0}};
 m10.reward={55000,5000,"Mission 11 - The Facility"}; s.registerMission(m10);

 MissionDefinition m11; m11.id="story_11_the_facility"; m11.title="The Facility"; m11.category=MissionCategory::Story; m11.campaign=GameMode::Story; m11.prerequisite="story_10_follow_the_money"; m11.chapter=3;
 m11.objectives={
  {ObjectiveType::ReachLocation,"Reach the hidden rural facility",1,0},
  {ObjectiveType::Interact,"Find a way inside the facility",1,0},
  {ObjectiveType::ReachLocation,"Reach the Project Sight operations room",1,0},
  {ObjectiveType::Interact,"Download Project Sight target maps",1,0},
  {ObjectiveType::Interact,"Recover files on Dash, Mayja and Rhys",1,0},
  {ObjectiveType::Interact,"Identify the director behind Project Sight",1,0},
  {ObjectiveType::EscapeWanted,"Escape the facility with the archive",1,0}
 };
 m11.reward={70000,6500,"Mission 12 - Phase II"}; s.registerMission(m11);

 MissionDefinition m12; m12.id="story_12_phase_ii"; m12.title="Phase II"; m12.category=MissionCategory::Story; m12.campaign=GameMode::Story; m12.prerequisite="story_11_the_facility"; m12.chapter=3;
 m12.objectives={
  {ObjectiveType::Interact,"Review the Project Sight 72-hour countdown",1,0},
  {ObjectiveType::Interact,"Disrupt the first Phase II target",1,0},
  {ObjectiveType::Interact,"Disrupt the second Phase II target",1,0},
  {ObjectiveType::ReachLocation,"Return to Downtown during the lockdown",1,0},
  {ObjectiveType::Interact,"Identify Elias Kane",1,0},
  {ObjectiveType::Interact,"Survive Vossler's lockdown threat",1,0}
 };
 m12.reward={90000,8000,"Mission 13 - Kane"}; s.registerMission(m12);

 MissionDefinition m13; m13.id="story_13_kane"; m13.title="Kane"; m13.category=MissionCategory::Story; m13.campaign=GameMode::Story; m13.prerequisite="story_12_phase_ii"; m13.chapter=3;
 m13.objectives={
  {ObjectiveType::ReachLocation,"Track Elias Kane through Sight City",1,0},
  {ObjectiveType::ReachLocation,"Reach Kane's security convoy",1,0},
  {ObjectiveType::Interact,"Disable Kane's escort without killing him",1,0},
  {ObjectiveType::Interact,"Capture Elias Kane alive",1,0},
  {ObjectiveType::Interact,"Interrogate Kane about Project Sight",1,0},
  {ObjectiveType::Interact,"Learn the truth about Rhys",1,0},
  {ObjectiveType::EscapeWanted,"Escape with Kane's Project Sight intelligence",1,0}
 };
 m13.reward={110000,9500,"Mission 14 - No Safe Place"}; s.registerMission(m13);

 MissionDefinition m14; m14.id="story_14_no_safe_place"; m14.title="No Safe Place"; m14.category=MissionCategory::Story; m14.campaign=GameMode::Story; m14.prerequisite="story_13_kane"; m14.chapter=3;
 m14.objectives={
  {ObjectiveType::ReachLocation,"Return to the crew safehouse",1,0},
  {ObjectiveType::Interact,"Survive the Project Sight safehouse assault",1,0},
  {ObjectiveType::EnterVehicle,"Escape in the crew vehicle",1,0},
  {ObjectiveType::EscapeWanted,"Survive the rolling gunfight",1,0},
  {ObjectiveType::EscapeWanted,"Evade the police helicopter at Wanted Level 4",1,0},
  {ObjectiveType::ReachLocation,"Reach the old hideout",1,0}
 };
 m14.reward={125000,11000,"Mission 15 - The Hostage"}; s.registerMission(m14);

 MissionDefinition m15; m15.id="story_15_the_hostage"; m15.title="The Hostage"; m15.category=MissionCategory::Story; m15.campaign=GameMode::Story; m15.prerequisite="story_14_no_safe_place"; m15.chapter=3;
 m15.objectives={
  {ObjectiveType::ReachLocation,"Reach the Downtown construction hotel",1,0},
  {ObjectiveType::Interact,"Choose an entry route into the hotel",1,0},
  {ObjectiveType::Interact,"Locate and rescue the captured crew member",1,0},
  {ObjectiveType::Interact,"Protect the hostage during the extraction",1,0},
  {ObjectiveType::EscapeWanted,"Escape through the parking garage",1,0},
  {ObjectiveType::Interact,"Recover intelligence on THE CLEANUP",1,0}
 };
 m15.reward={145000,13000,"Mission 16 - The Cleanup"}; s.registerMission(m15);

 MissionDefinition m16; m16.id="story_16_the_cleanup"; m16.title="The Cleanup"; m16.category=MissionCategory::Story; m16.campaign=GameMode::Story; m16.prerequisite="story_15_the_hostage"; m16.chapter=4;
 m16.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja and review Project Sight cleanup targets",1,0},
  {ObjectiveType::Interact,"Choose the cleanup target",1,0},
  {ObjectiveType::Interact,"Neutralize the chosen Project Sight cleanup operation",1,0},
  {ObjectiveType::EscapeWanted,"Escape Project Sight retaliation",1,0},
  {ObjectiveType::Interact,"Recover the Master Ledger",1,0}
 };
 m16.reward={175000,15000,"Mission 17 - The Ledger"}; s.registerMission(m16);

 MissionDefinition m17; m17.id="story_17_the_ledger"; m17.title="The Ledger"; m17.category=MissionCategory::Story; m17.campaign=GameMode::Story; m17.prerequisite="story_16_the_cleanup"; m17.chapter=4;
 m17.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja and review the Master Ledger targets",1,0},
  {ObjectiveType::Interact,"Raid the Harbor / Port ledger node",1,0},
  {ObjectiveType::Interact,"Raid the Airport ledger node",1,0},
  {ObjectiveType::Interact,"Raid the South Hills ledger node",1,0},
  {ObjectiveType::Interact,"Trace the Downtown money trail",1,0},
  {ObjectiveType::Deliver,"Deliver the complete ledger package to Mayja",1,0}
 };
 m17.reward={210000,18000,"Operation Blackout"}; s.registerMission(m17);

 MissionDefinition m18; m18.id="story_18_operation_blackout"; m18.title="Operation Blackout"; m18.category=MissionCategory::Story; m18.campaign=GameMode::Story; m18.prerequisite="story_17_the_ledger"; m18.chapter=4;
 m18.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja and review the four Blackout operations",1,0},
  {ObjectiveType::Interact,"Choose one Blackout operation",1,0},
  {ObjectiveType::Interact,"Execute the selected Blackout operation",1,0},
  {ObjectiveType::EscapeWanted,"Break Project Sight response and regroup",1,0},
  {ObjectiveType::Interact,"Confirm Central Reserve access window",1,0}
 };
 m18.reward={250000,20000,"Mission 19 - Central Reserve Heist"}; s.registerMission(m18);

 MissionDefinition m19; m19.id="story_19_central_reserve_heist"; m19.title="Central Reserve Heist"; m19.category=MissionCategory::Heist; m19.campaign=GameMode::Story; m19.prerequisite="story_18_operation_blackout"; m19.chapter=5;
 m19.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja in the Heist Room and review the $900,000,000 target",1,0},
  {ObjectiveType::Interact,"Choose the heist approach: Quiet Entry / Underground / Direct Assault",1,0},
  {ObjectiveType::Interact,"Complete Weapons setup",1,0},
  {ObjectiveType::Interact,"Complete Getaway Vehicles setup",1,0},
  {ObjectiveType::Interact,"Complete Ammo setup",1,0},
  {ObjectiveType::Interact,"Confirm crew, escape route and START HEIST",1,0},
  {ObjectiveType::Interact,"Breach Sight City Central Reserve and secure the score",1,0},
  {ObjectiveType::EscapeWanted,"Escape the Central Reserve response at Wanted Level 7",1,0},
  {ObjectiveType::Deliver,"Deliver the $900,000,000 score and regroup with the crew",1,0}
 };
 m19.reward={0,100000,"Central Reserve aftermath"}; s.registerMission(m19);

 MissionDefinition m20; m20.id="story_20_central_reserve_aftermath"; m20.title="Central Reserve Aftermath"; m20.category=MissionCategory::Story; m20.campaign=GameMode::Story; m20.prerequisite="story_19_central_reserve_heist"; m20.chapter=5;
 m20.objectives={
  {ObjectiveType::EscapeWanted,"Break the combined police and Project Sight post-heist dragnet",1,0},
  {ObjectiveType::Deliver,"Secure the surviving Central Reserve money vehicles",1,0},
  {ObjectiveType::Interact,"Allocate the $900,000,000 crew cuts",1,0},
  {ObjectiveType::Interact,"Receive Vossler's post-heist threat",1,0},
  {ObjectiveType::ReachLocation,"Regroup with the crew and account for survivors, cuts and vehicles",1,0}
 };
 m20.reward={250000,25000,"Final battle preparation"}; s.registerMission(m20);

 MissionDefinition m21; m21.id="story_21_final_battle_preparation"; m21.title="Road to Fortune"; m21.category=MissionCategory::Story; m21.campaign=GameMode::Story; m21.prerequisite="story_20_central_reserve_aftermath"; m21.chapter=5;
 m21.objectives={
  {ObjectiveType::ReachLocation,"Meet Mayja at the final-war staging area",1,0},
  {ObjectiveType::Interact,"Gather Ammo supplies",1,0},
  {ObjectiveType::Interact,"Gather Weapons supplies",1,0},
  {ObjectiveType::Interact,"Gather Cars and getaway vehicles",1,0},
  {ObjectiveType::Interact,"Gather Armor supplies",1,0},
  {ObjectiveType::Interact,"Gather Heavy resources",1,0},
  {ObjectiveType::Interact,"Review the physical supplies board and choose the crew",1,0},
  {ObjectiveType::Interact,"Confirm I’M READY for TAKE SIGHT CITY",1,0}
 };
 m21.reward={500000,50000,"TAKE SIGHT CITY"}; s.registerMission(m21);

 MissionDefinition m22; m22.id="story_22_take_sight_city"; m22.title="TAKE SIGHT CITY"; m22.category=MissionCategory::Story; m22.campaign=GameMode::Story; m22.prerequisite="story_21_final_battle_preparation"; m22.chapter=5;
 m22.objectives={{ObjectiveType::ReachLocation,"Begin the final assault and choose an outer territory",1,0},{ObjectiveType::Interact,"Capture outer territory 1",1,0},{ObjectiveType::Interact,"Capture outer territory 2",1,0},{ObjectiveType::Interact,"Capture outer territory 3",1,0},{ObjectiveType::Interact,"Capture outer territory 4",1,0},{ObjectiveType::Interact,"Capture outer territory 5 and break the outer defenses",1,0},{ObjectiveType::ReachLocation,"Assault Downtown",1,0},{ObjectiveType::Interact,"Defeat Vossler's Downtown forces",1,0},{ObjectiveType::ReachLocation,"Chase Adrian Vossler",1,0},{ObjectiveType::Interact,"Defeat Adrian Vossler",1,0},{ObjectiveType::Interact,"Complete the Story and take control of Sight City",1,0}};
 m22.reward={1000000,100000,"Story Complete / Free Roam"}; s.registerMission(m22);
}
void registerOnlineCampaign(MissionSystem& s){
 s.registerMission(CM("online_01_newcomer","New to Sight City",MissionCategory::CoopOnline,GameMode::Online,ObjectiveType::ReachLocation,ObjectiveType::Interact,1500,120,"Online Jobs","",1));
 s.registerMission(CM("online_02_crew_run","Crew Supply Run",MissionCategory::CoopOnline,GameMode::Online,ObjectiveType::CoopSync,ObjectiveType::Deliver,3200,220,"Online Businesses","online_01_newcomer",1));
 s.registerMission(CM("online_race_city","Sight City Online Circuit",MissionCategory::Race,GameMode::Online,ObjectiveType::ReachLocation,ObjectiveType::WinRace,4000,280,"Online Racing","online_02_crew_run",2));
 s.registerMission(CM("online_heist_bank","Online Bank Job",MissionCategory::Heist,GameMode::Online,ObjectiveType::CoopSync,ObjectiveType::EscapeWanted,11000,600,"Online Heists","online_race_city",3));
 s.registerMission(CM("online_05_business","Empire Starter",MissionCategory::Business,GameMode::Online,ObjectiveType::Interact,ObjectiveType::Deliver,7000,450,"Online Business Tier 2","online_heist_bank",4));
 s.registerMission(CM("online_06_crew_heist","Crew Finale",MissionCategory::Heist,GameMode::Online,ObjectiveType::CoopSync,ObjectiveType::EscapeWanted,20000,1200,"Online Free Roam+","online_05_business",5));
}
