#include "StoryGameplay.h"
#include <algorithm>

StoryGameplay::StoryGameplay() : emergency_(false) {
    dash_=makeStoryCharacter();
    registerStoryCampaign(missions_);
    rhysCar_.position={6,0,4};
    deliveryCar_.position={32,0,18};
    containerTruck_.position={-1800,0,-2400};
    escapeCar_.position={-1200,0,-900};
    heistGetawayCar_.position={-600,0,350};
    syncHud("Welcome to Sight City: Underworld");
}

bool StoryGameplay::startMission1(){
    if(!missions_.start("story_01_back_in_sight_city",GameMode::Story)) return false;
    syncHud("Meet Rhys at the arrival point");
    return true;
}

bool StoryGameplay::advance(ObjectiveType type,const char* notification,bool checkpoint){
    if(!missions_.addProgress(type,1)) return false;
    if(checkpoint && missions_.activeMission()) missions_.setCheckpoint();
    syncHud(notification);
    return true;
}

bool StoryGameplay::reachRhys(){ return advance(ObjectiveType::ReachLocation,"Rhys: Good to see you, Dash."); }
bool StoryGameplay::enterRhysVehicle(){ return advance(ObjectiveType::EnterVehicle,"Get in. I'll show you what changed."); }
bool StoryGameplay::reachDowntown(){ return advance(ObjectiveType::ReachLocation,"Downtown discovered"); }
bool StoryGameplay::reachWestSuburbs(){ return advance(ObjectiveType::ReachLocation,"West Suburbs discovered"); }
bool StoryGameplay::reachEastCoast(){ return advance(ObjectiveType::ReachLocation,"East Coast discovered"); }
bool StoryGameplay::reachNorthCountryside(){ return advance(ObjectiveType::ReachLocation,"North Countryside discovered"); }
bool StoryGameplay::reachMountRidge(){ return advance(ObjectiveType::ReachLocation,"Mount Ridge discovered"); }
bool StoryGameplay::reachNightCity(){ time_.setTime(21,0); return advance(ObjectiveType::ReachLocation,"Sight City after dark"); }
bool StoryGameplay::completeTour(){
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Back in Sight City",false);
    if(ok && mission1Complete()){
        dash_.addMoney(5000); dash_.addXp(500);
        syncHud("Mission 2 unlocked: Like Old Times");
    }
    return ok;
}
bool StoryGameplay::mission1Complete()const{return missions_.stateOf("story_01_back_in_sight_city")==MissionState::Completed;}

bool StoryGameplay::startMission2(){
    if(!mission1Complete()) return false;
    if(!missions_.start("story_02_like_old_times",GameMode::Story)) return false;
    gangChaseActive_=false; gangChasePressure_=0.0f;
    syncHud("Rhys: Meet me at the old garage.");
    return true;
}
bool StoryGameplay::reachOldGarage(){ return advance(ObjectiveType::ReachLocation,"Rhys: There she is. Still runs, somehow."); }
bool StoryGameplay::enterDeliveryCar(){
    player_.position=deliveryCar_.position;
    player_.tryEnterVehicle(deliveryCar_,3.5f);
    return advance(ObjectiveType::EnterVehicle,"Deliver the car across town.");
}
bool StoryGameplay::deliverCar(){
    const bool ok=advance(ObjectiveType::ReachLocation,"Delivery made. Rival gang vehicles incoming!",false);
    if(ok) gangChaseStarts();
    return ok;
}
bool StoryGameplay::gangChaseStarts(){
    gangChaseActive_=true;
    gangChasePressure_=100.0f;
    hud_.chasePressure=100;
    hud_.notification="RIVAL GANG CHASE - Lose them";
    return true;
}
bool StoryGameplay::loseRivalGang(){
    if(!gangChaseActive_) return false;
    gangChasePressure_=0.0f;
    gangChaseActive_=false;
    hud_.chasePressure=0;
    return advance(ObjectiveType::EscapeWanted,"Gang lost. Head for the old workshop.");
}
bool StoryGameplay::reachOldWorkshop(){ return advance(ObjectiveType::ReachLocation,"Old workshop reached"); }
bool StoryGameplay::completeMission2(){
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Like Old Times",false);
    if(ok && mission2Complete()){
        dash_.addMoney(8000); dash_.addXp(650);
        syncHud("Rhys: Just like old times. You drive, I find the jobs. Mission 3 unlocked: Easy Money");
    }
    return ok;
}
bool StoryGameplay::mission2Complete()const{return missions_.stateOf("story_02_like_old_times")==MissionState::Completed;}

bool StoryGameplay::failAndRetry(){
    if(!missions_.fail()) return false;
    const bool ok=missions_.restartFromCheckpoint();
    gangChaseActive_=false; gangChasePressure_=0.0f; hud_.chasePressure=0;
    syncHud(ok?"Checkpoint restored":"Retry failed");
    return ok;
}
void StoryGameplay::update(float dt){
    time_.update(dt);
    if(gangChaseActive_){
        // Runtime hook: missions can lower this while Dash gains distance/line-of-sight breaks.
        gangChasePressure_=std::max(0.0f,gangChasePressure_-dt*2.0f);
        hud_.chasePressure=(int)gangChasePressure_;
    }
    (void)weather_; (void)wanted_; (void)emergency_;
}
void StoryGameplay::syncHud(const char* notification){
    const auto*m=missions_.activeMission(); const auto*p=missions_.activeProgress();
    hud_.missionTitle=m?m->title:(mission8Complete()?"Behind My Back":(mission7Complete()?"Wrong Place, Wrong Time":(mission6Complete()?"Something\'s Off":(mission5Complete()?"Our First Score":(mission4Complete()?"The Long Way Home":(mission3Complete()?"Easy Money":(mission2Complete()?"Like Old Times":(mission1Complete()?"Back in Sight City":"Story Mode"))))))));
    hud_.objectiveText=(m&&p&&p->objectiveIndex<m->objectives.size())?m->objectives[p->objectiveIndex].text:"";
    if(notification) hud_.notification=notification;
    hud_.money=dash_.money; hud_.xp=dash_.xp; hud_.checkpoint=p?(int)p->checkpointObjective:0;
    hud_.chasePressure=(int)gangChasePressure_;
}

bool StoryGameplay::startMission3(){
    if(!mission2Complete()) return false;
    if(!missions_.start("story_03_easy_money",GameMode::Story)) return false;
    gangChaseActive_=false; gangChasePressure_=0.0f;
    syncHud("Rhys: Harbor Docks. We grab the electronics and get out."); return true;
}
bool StoryGameplay::reachHarborDocks(){return advance(ObjectiveType::ReachLocation,"Harbor Docks reached. Find the marked container.");}
bool StoryGameplay::locateElectronicsContainer(){return advance(ObjectiveType::Interact,"Container located. Take the truck.");}
bool StoryGameplay::enterContainerTruck(){
    player_.position=containerTruck_.position; player_.tryEnterVehicle(containerTruck_,3.5f);
    const bool ok=advance(ObjectiveType::EnterVehicle,"Rhys: They knew the route. Rival vehicles incoming!",false);
    if(ok) gangChaseStarts(); return ok;
}
bool StoryGameplay::loseHarborRivals(){
    if(!gangChaseActive_) return false; gangChaseActive_=false; gangChasePressure_=0; hud_.chasePressure=0;
    return advance(ObjectiveType::EscapeWanted,"Rivals lost. Deliver the electronics.");
}
bool StoryGameplay::deliverElectronics(){
    const bool ok=advance(ObjectiveType::Deliver,"MISSION COMPLETE - Easy Money",false);
    if(ok&&mission3Complete()){dash_.addMoney(12500);dash_.addXp(800);syncHud("$12,500 + 800 XP - Mission 4 unlocked: The Long Way Home");} return ok;
}
bool StoryGameplay::mission3Complete()const{return missions_.stateOf("story_03_easy_money")==MissionState::Completed;}
DynamicIncident StoryGameplay::spawnDynamicIncident(DynamicIncidentType type){return incidents_.spawn(type,player_.position);}
MeetingMarker StoryGameplay::characterMeeting(const std::string& character)const{return meetings_.markerFor(character,time_.hour());}


bool StoryGameplay::startMission4(){
    if(!mission3Complete()) return false;
    if(!missions_.start("story_04_long_way_home",GameMode::Story)) return false;
    player_.position=escapeCar_.position;
    player_.tryEnterVehicle(escapeCar_,3.5f);
    gangChaseActive_=true; gangChasePressure_=100.0f;
    wanted_.level=2; wanted_.heat=35.0f;
    syncHud("Rhys: Move! Industrial Zone is crawling with them.");
    return true;
}
bool StoryGameplay::escapeIndustrialZone(){
    gangChasePressure_=85.0f; hud_.chasePressure=85;
    return advance(ObjectiveType::ReachLocation,"Escape Industrial Zone - take the highway north.");
}
bool StoryGameplay::reachHighway(){
    wanted_.level=3; wanted_.heat=55.0f;
    gangChasePressure_=70.0f; hud_.chasePressure=70;
    return advance(ObjectiveType::ReachLocation,"Police joined the chase. Reach the mountain tunnels.");
}
bool StoryGameplay::enterMountainTunnels(){
    wanted_.level=4; wanted_.heat=75.0f;
    gangChasePressure_=55.0f; hud_.chasePressure=55;
    return advance(ObjectiveType::ReachLocation,"Inside the tunnels. Break line of sight and lose the pursuit.");
}
bool StoryGameplay::survivePolicePursuit(){
    wanted_.clear();
    gangChaseActive_=false; gangChasePressure_=0.0f; hud_.chasePressure=0;
    return advance(ObjectiveType::EscapeWanted,"Pursuit lost. Head to the North Countryside garage.");
}
bool StoryGameplay::reachNorthCountrysideGarage(){
    const bool ok=advance(ObjectiveType::ReachLocation,"MISSION COMPLETE - The Long Way Home",false);
    if(ok&&mission4Complete()){
        dash_.addMoney(18000); dash_.addXp(1200);
        syncHud("$18,000 + 1,200 XP - Mountain garage unlocked - Mission 5 unlocked: Our First Score");
    }
    return ok;
}
bool StoryGameplay::mission4Complete()const{return missions_.stateOf("story_04_long_way_home")==MissionState::Completed;}


bool StoryGameplay::startMission5(){
    if(!mission4Complete()) return false;
    if(!missions_.start("story_05_our_first_score",GameMode::Story)) return false;
    storyHeist_.reset(); wanted_.clear();
    syncHud("Rhys: Meet Mayja. She has a real score for us.");
    return true;
}
bool StoryGameplay::meetMayjaAtHeistRoom(){
    if(!storyHeist_.meetMayja()) return false;
    return advance(ObjectiveType::ReachLocation,"Mayja: Armored transport. We plan this properly, nobody improvises.");
}
bool StoryGameplay::chooseGetawayVehicle(const std::string&id,bool owned,bool compatible,bool locked){
    if(!storyHeist_.selectGetawayVehicle(id,owned,compatible,locked)) { syncHud("Mayja: We need an owned, heist-compatible vehicle."); return false; }
    return advance(ObjectiveType::Interact,"GETAWAY VEHICLE READY");
}
bool StoryGameplay::collectHeistEquipment(){
    if(!storyHeist_.collectEquipment()) return false;
    return advance(ObjectiveType::Interact,"HEIST EQUIPMENT READY");
}
bool StoryGameplay::chooseHeistEscapeRoute(EscapeRoute route){
    if(!storyHeist_.chooseEscapeRoute(route)) return false;
    return advance(ObjectiveType::Interact,"Mayja: Route locked. Move on the transport.");
}
bool StoryGameplay::reachArmoredTransport(){ return advance(ObjectiveType::ReachLocation,"Armored transport located. Take the score."); }
bool StoryGameplay::takeArmoredTransportScore(){
    if(!storyHeist_.takeScore()) return false;
    wanted_.level=3; wanted_.heat=60.0f;
    return advance(ObjectiveType::Interact,"SCORE SECURED - Police responding.");
}
bool StoryGameplay::enterHeistGetawayVehicle(){
    player_.position=heistGetawayCar_.position; player_.tryEnterVehicle(heistGetawayCar_,3.5f);
    wanted_.level=4; wanted_.heat=80.0f;
    return advance(ObjectiveType::EnterVehicle,"Mayja: Go! Follow the escape route.");
}
bool StoryGameplay::escapeFirstHeist(){
    if(!storyHeist_.markEscaped()) return false;
    wanted_.clear();
    return advance(ObjectiveType::EscapeWanted,"Police lost. Deliver the score to the safehouse.");
}
bool StoryGameplay::deliverFirstScore(){
    if(!storyHeist_.deliverScore()) return false;
    const bool ok=advance(ObjectiveType::Deliver,"HEIST COMPLETE - Our First Score",false);
    if(ok&&mission5Complete()){
        dash_.addMoney(75000); dash_.addXp(3000);
        syncHud("$75,000 + 3,000 XP - HEISTS UNLOCKED - Mission 6 unlocked: Something's Off");
    }
    return ok;
}
bool StoryGameplay::mission5Complete()const{return missions_.stateOf("story_05_our_first_score")==MissionState::Completed;}


bool StoryGameplay::startMission6(){
    if(!mission5Complete()) return false;
    if(!missions_.start("story_06_somethings_off",GameMode::Story)) return false;
    rhysFollowed_=false; mayjaLeakDetected_=false; wanted_.clear();
    syncHud("Mayja: Rhys missed the planning session. That is not like him.");
    return true;
}
bool StoryGameplay::meetMayjaAfterFirstScore(){
    return advance(ObjectiveType::ReachLocation,"Mayja: We have another job. Rhys should be here.");
}
bool StoryGameplay::reviewNextJobWithoutRhys(){
    return advance(ObjectiveType::Interact,"Mayja: Only three people knew this route. Keep your eyes open.");
}
bool StoryGameplay::findRhysNearRivalDistrict(){
    return advance(ObjectiveType::ReachLocation,"Rhys spotted near rival territory. Stay back.");
}
bool StoryGameplay::observeRhysFromDistance(){
    return advance(ObjectiveType::Interact,"OPTIONAL: Follow Rhys and see where he goes.");
}
bool StoryGameplay::followRhysOptional(){
    rhysFollowed_=true;
    return advance(ObjectiveType::ReachLocation,"You followed Rhys. He met someone connected to the rival gang.");
}
bool StoryGameplay::reportSuspicionToMayja(){
    mayjaLeakDetected_=true;
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Something's Off",false);
    if(ok&&mission6Complete()){
        dash_.addMoney(30000); dash_.addXp(2500);
        syncHud("$30,000 + 2,500 XP - Mayja: We have a leak. Mission 7 unlocked: Wrong Place, Wrong Time");
    }
    return ok;
}
bool StoryGameplay::mission6Complete()const{return missions_.stateOf("story_06_somethings_off")==MissionState::Completed;}

bool StoryGameplay::startMission7(){
    if(!mission6Complete()) return false;
    if(!missions_.start("story_07_wrong_place_wrong_time",GameMode::Story)) return false;
    rivalAmbushActive_=false;
    rhysRescueTriggered_=false;
    insiderConfirmed_=false;
    gangChaseActive_=false;
    gangChasePressure_=0.0f;
    wanted_.clear();
    syncHud("Rhys: Industrial Zone. Simple pickup. In and out.");
    return true;
}
bool StoryGameplay::meetRhysForIndustrialJob(){
    return advance(ObjectiveType::ReachLocation,"Rhys: Job should be quiet. Stay close.");
}
bool StoryGameplay::enterIndustrialJobSite(){
    return advance(ObjectiveType::ReachLocation,"The site is too quiet. Check the loading yard.");
}
bool StoryGameplay::discoverRivalAmbush(){
    rivalAmbushActive_=true;
    gangChaseActive_=true;
    gangChasePressure_=100.0f;
    hud_.chasePressure=100;
    return advance(ObjectiveType::Interact,"AMBUSH! Rival gang units were already waiting for you.");
}
bool StoryGameplay::surviveRivalAmbush(){
    if(!rivalAmbushActive_) return false;
    gangChasePressure_=72.0f;
    hud_.chasePressure=72;
    return advance(ObjectiveType::Interact,"Hold the yard. Rhys is moving to your position.");
}
bool StoryGameplay::rhysSavesDash(){
    if(!rivalAmbushActive_) return false;
    rhysRescueTriggered_=true;
    gangChasePressure_=48.0f;
    hud_.chasePressure=48;
    return advance(ObjectiveType::Interact,"Rhys: Dash! Move! I have you covered.");
}
bool StoryGameplay::escapeIndustrialAmbush(){
    if(!rhysRescueTriggered_) return false;
    rivalAmbushActive_=false;
    gangChaseActive_=false;
    gangChasePressure_=0.0f;
    hud_.chasePressure=0;
    return advance(ObjectiveType::EscapeWanted,"Rivals lost. Meet Mayja at the safehouse.");
}
bool StoryGameplay::debriefMayjaAfterAmbush(){
    insiderConfirmed_=true;
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Wrong Place, Wrong Time",false);
    if(ok&&mission7Complete()){
        dash_.addMoney(35000); dash_.addXp(3000);
        syncHud("$35,000 + 3,000 XP - Mayja: Only three people had that location. We have an insider. Mission 8 unlocked: Behind My Back");
    }
    return ok;
}
bool StoryGameplay::mission7Complete()const{return missions_.stateOf("story_07_wrong_place_wrong_time")==MissionState::Completed;}


bool StoryGameplay::startMission8(){
    if(!mission7Complete()) return false;
    if(!missions_.start("story_08_behind_my_back",GameMode::Story)) return false;
    rhysMeetingConfirmed_=false;
    betrayalEvidenceSecured_=false;
    wanted_.clear();
    syncHud("Mayja: We need proof. Follow Rhys, but do not let him see you.");
    return true;
}
bool StoryGameplay::meetMayjaForSurveillance(){
    return advance(ObjectiveType::ReachLocation,"Mayja: Keep your distance. We need to know who he is meeting.");
}
bool StoryGameplay::tailRhysUndetected(){
    return advance(ObjectiveType::ReachLocation,"Rhys is heading deeper into rival territory. Stay out of sight.");
}
bool StoryGameplay::captureRhysMeetingEvidence(){
    rhysMeetingConfirmed_=true;
    betrayalEvidenceSecured_=true;
    return advance(ObjectiveType::Interact,"EVIDENCE SECURED - Rhys is meeting the rival gang leader.");
}
bool StoryGameplay::leaveRivalMeetingArea(){
    if(!betrayalEvidenceSecured_) return false;
    return advance(ObjectiveType::ReachLocation,"Leave the area quietly. Take the evidence to Mayja.");
}
bool StoryGameplay::showRhysEvidenceToMayja(){
    if(!betrayalEvidenceSecured_) return false;
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Behind My Back",false);
    if(ok&&mission8Complete()){
        dash_.addMoney(38000); dash_.addXp(3500);
        syncHud("$38,000 + 3,500 XP - Mayja: Now we know. Mission 9 unlocked: Betrayed");
    }
    return ok;
}
bool StoryGameplay::mission8Complete()const{return missions_.stateOf("story_08_behind_my_back")==MissionState::Completed;}


// Mission 9 - Betrayed
bool StoryGameplay::startMission9(){
    if(!mission8Complete()) return false;
    if(!missions_.start("story_09_betrayed",GameMode::Story)) return false;
    fakeHeistTrapActive_=false; rhysBetrayalConfirmed_=false; biggerPlayerRevealed_=false;
    wanted_.clear();
    syncHud("Mayja: We give them a job that does not exist. If they move, we have our answer.");
    return true;
}
bool StoryGameplay::briefFakeHeistTrap(){
    return advance(ObjectiveType::ReachLocation,"Mayja: Only you, me and Rhys get this location. Nobody else.");
}
bool StoryGameplay::plantFakeHeistInformation(){
    fakeHeistTrapActive_=true;
    return advance(ObjectiveType::Interact,"Fake heist information planted. Watch the target location.");
}
bool StoryGameplay::observeRivalsTakeTheBait(){
    if(!fakeHeistTrapActive_) return false;
    rhysBetrayalConfirmed_=true;
    return advance(ObjectiveType::Interact,"The rival gang moved on the exact fake location. The leak is confirmed.");
}
bool StoryGameplay::confrontRhys(){
    if(!rhysBetrayalConfirmed_) return false;
    return advance(ObjectiveType::ReachLocation,"Dash: No more lies, Rhys. Tell me why.");
}
bool StoryGameplay::hearRhysConfession(){
    if(!rhysBetrayalConfirmed_) return false;
    return advance(ObjectiveType::Interact,"Rhys: I wanted a way to the top. I was stuck in your shadow... and I needed a way out.");
}
bool StoryGameplay::learnAboutBiggerPlayer(){
    if(!rhysBetrayalConfirmed_) return false;
    biggerPlayerRevealed_=true;
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Betrayed",false);
    if(ok&&mission9Complete()){
        dash_.addMoney(40000); dash_.addXp(4000);
        syncHud("$40,000 + 4,000 XP - Rhys: The gang was not the real buyer. Someone bigger paid for everything. Mission 10 unlocked: Follow the Money");
    }
    return ok;
}
bool StoryGameplay::mission9Complete()const{return missions_.stateOf("story_09_betrayed")==MissionState::Completed;}

// Mission 10 - Follow the Money
bool StoryGameplay::startMission10(){ if(!mission9Complete()||!biggerPlayerRevealed_)return false; if(!missions_.start("story_10_follow_the_money",GameMode::Story))return false; vosslerHoldingsIdentified_=false; projectSightDiscovered_=false; syncHud("Mayja: We follow every payment. Start in the Industrial Zone."); return true; }
bool StoryGameplay::traceIndustrialPayment(){return advance(ObjectiveType::ReachLocation,"Industrial Zone - payment route located");}
bool StoryGameplay::recoverPaymentRecord(){return advance(ObjectiveType::Interact,"Mayja: Shell company. The money keeps moving.");}
bool StoryGameplay::followCourierDowntown(){return advance(ObjectiveType::ReachLocation,"Courier tracked through Downtown");}
bool StoryGameplay::trackTransferToAirport(){return advance(ObjectiveType::ReachLocation,"Airport transfer identified");}
bool StoryGameplay::followMoneyToWestCoast(){return advance(ObjectiveType::ReachLocation,"West Coast - final account relay located");}
bool StoryGameplay::identifyProjectSight(){vosslerHoldingsIdentified_=projectSightDiscovered_=true; bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Follow the Money",false); if(ok&&mission10Complete()){dash_.addMoney(55000);dash_.addXp(5000);syncHud("$55,000 + 5,000 XP - Vossler Holdings. Project Sight. Mission 11 unlocked: The Facility");}return ok;}
bool StoryGameplay::mission10Complete()const{return missions_.stateOf("story_10_follow_the_money")==MissionState::Completed;}


// Mission 11 - The Facility
bool StoryGameplay::startMission11(){
    if(!mission10Complete()||!projectSightDiscovered_) return false;
    if(!missions_.start("story_11_the_facility",GameMode::Story)) return false;
    facilityInfiltrated_=false;
    projectSightArchiveSecured_=false;
    crewDossiersRecovered_=false;
    adrianVosslerNamed_=false;
    wanted_.clear();
    syncHud("Mayja: The money trail ends at a rural facility. Get inside and find out what Project Sight is building.");
    return true;
}
bool StoryGameplay::reachHiddenFacility(){
    return advance(ObjectiveType::ReachLocation,"Hidden facility located in the countryside. Security is heavier than expected.");
}
bool StoryGameplay::infiltrateFacility(){
    facilityInfiltrated_=true;
    return advance(ObjectiveType::Interact,"Facility breached. Find the Project Sight operations room.");
}
bool StoryGameplay::reachProjectSightOperationsRoom(){
    if(!facilityInfiltrated_) return false;
    return advance(ObjectiveType::ReachLocation,"PROJECT SIGHT OPERATIONS - restricted network located.");
}
bool StoryGameplay::downloadProjectSightTargetMaps(){
    if(!facilityInfiltrated_) return false;
    projectSightArchiveSecured_=true;
    return advance(ObjectiveType::Interact,"Archive downloaded: police routes, businesses, banks, the Port, Airport, gangs and transport networks are all mapped.");
}
bool StoryGameplay::recoverCrewDossiers(){
    if(!projectSightArchiveSecured_) return false;
    crewDossiersRecovered_=true;
    return advance(ObjectiveType::Interact,"Personal dossiers recovered: DASH / MAYJA / RHYS. Project Sight has been tracking the crew.");
}
bool StoryGameplay::identifyAdrianVossler(){
    if(!crewDossiersRecovered_) return false;
    adrianVosslerNamed_=true;
    return advance(ObjectiveType::Interact,"Mayja: Adrian Vossler. Founder of Vossler Holdings. He is running Project Sight.");
}
bool StoryGameplay::escapeFacilityWithArchive(){
    if(!adrianVosslerNamed_||!projectSightArchiveSecured_) return false;
    wanted_.level=3; wanted_.heat=55.0f;
    const bool ok=advance(ObjectiveType::EscapeWanted,"MISSION COMPLETE - The Facility",false);
    wanted_.clear();
    if(ok&&mission11Complete()){
        dash_.addMoney(70000); dash_.addXp(6500);
        syncHud("$70,000 + 6,500 XP - Project Sight archive secured. Mission 12 unlocked: Phase II");
    }
    return ok;
}
bool StoryGameplay::mission11Complete()const{return missions_.stateOf("story_11_the_facility")==MissionState::Completed;}


// Mission 12 - Phase II
bool StoryGameplay::startMission12(){
    if(!mission11Complete()||!adrianVosslerNamed_) return false;
    if(!missions_.start("story_12_phase_ii",GameMode::Story)) return false;
    phaseIICountdownReviewed_=phaseIIPowerDone_=phaseIITransportDone_=phaseIIMoneyDone_=false;
    phaseIILockdownActive_=eliasKaneIdentified_=false; phaseIITargetsCompleted_=0;
    syncHud("Mayja: Project Sight has started a 72-hour countdown. Power. Transport. Money. We can stop two before the city locks down.");
    return true;
}
bool StoryGameplay::reviewPhaseIICountdown(){ phaseIICountdownReviewed_=true; return advance(ObjectiveType::Interact,"PHASE II - 72 HOURS. Choose two targets: POWER / TRANSPORT / MONEY."); }
bool StoryGameplay::disruptPowerGrid(){ if(!phaseIICountdownReviewed_||phaseIIPowerDone_||phaseIITargetsCompleted_>=2)return false; phaseIIPowerDone_=true; ++phaseIITargetsCompleted_; return advance(ObjectiveType::Interact,"POWER disrupted. Project Sight grid control degraded."); }
bool StoryGameplay::disruptTransportNetwork(){ if(!phaseIICountdownReviewed_||phaseIITransportDone_||phaseIITargetsCompleted_>=2)return false; phaseIITransportDone_=true; ++phaseIITargetsCompleted_; return advance(ObjectiveType::Interact,"TRANSPORT disrupted. Project Sight routing network degraded."); }
bool StoryGameplay::disruptMoneyNetwork(){ if(!phaseIICountdownReviewed_||phaseIIMoneyDone_||phaseIITargetsCompleted_>=2)return false; phaseIIMoneyDone_=true; ++phaseIITargetsCompleted_; return advance(ObjectiveType::Interact,"MONEY disrupted. Project Sight financial relay degraded."); }
bool StoryGameplay::returnToDowntownLockdown(){ if(phaseIITargetsCompleted_!=2)return false; phaseIILockdownActive_=true; wanted_.level=4; wanted_.heat=75.0f; return advance(ObjectiveType::ReachLocation,"Downtown is in lockdown. Tactical units are sealing the streets."); }
bool StoryGameplay::identifyEliasKane(){ if(!phaseIILockdownActive_)return false; eliasKaneIdentified_=true; return advance(ObjectiveType::Interact,"Mayja: Elias Kane. Vossler's security chief. He is commanding the lockdown."); }
bool StoryGameplay::surviveVosslerLockdown(){
    if(!eliasKaneIdentified_)return false;
    const bool ok=advance(ObjectiveType::Interact,"MISSION COMPLETE - Phase II",false);
    wanted_.clear(); phaseIILockdownActive_=false;
    if(ok&&mission12Complete()){
        dash_.addMoney(90000); dash_.addXp(8000);
        syncHud("$90,000 + 8,000 XP - Vossler: To kill you. You, Mayja, Rhys... your entire crew. Rhys: Then come here and try. We're going to stop you. Dash: No. This is our chance. Mission 13 unlocked: Kane");
    }
    return ok;
}
bool StoryGameplay::mission12Complete()const{return missions_.stateOf("story_12_phase_ii")==MissionState::Completed;}


// Mission 13 - Kane
bool StoryGameplay::startMission13(){
    if(!mission12Complete()||!eliasKaneIdentified_) return false;
    if(!missions_.start("story_13_kane",GameMode::Story)) return false;
    kaneTracked_=kaneEscortDisabled_=kaneCapturedAlive_=kaneInterrogated_=false;
    rhysProjectSightHistoryRevealed_=vosslerIslandDataCenterKnown_=false;
    syncHud("MISSION 13 - KANE. Mayja: Find Kane. We need him alive.");
    return true;
}
bool StoryGameplay::trackEliasKane(){ kaneTracked_=true; return advance(ObjectiveType::ReachLocation,"Kane's encrypted security trail is moving toward the coast."); }
bool StoryGameplay::reachKaneSecurityConvoy(){ if(!kaneTracked_)return false; wanted_.level=2; wanted_.heat=35.0f; return advance(ObjectiveType::ReachLocation,"Kane's convoy located. Mayja: Do not kill Kane."); }
bool StoryGameplay::disableKaneEscort(){ if(!kaneTracked_)return false; kaneEscortDisabled_=true; wanted_.level=3; wanted_.heat=55.0f; return advance(ObjectiveType::Interact,"Escort disabled. Kane is trying to escape on foot."); }
bool StoryGameplay::captureKaneAlive(){ if(!kaneEscortDisabled_)return false; kaneCapturedAlive_=true; return advance(ObjectiveType::Interact,"ELIAS KANE CAPTURED ALIVE."); }
bool StoryGameplay::interrogateKane(){ if(!kaneCapturedAlive_)return false; kaneInterrogated_=true; return advance(ObjectiveType::Interact,"Kane: Vossler recruited Rhys before you ever came back to Sight City."); }
bool StoryGameplay::learnRhysProjectSightHistory(){
    if(!kaneInterrogated_)return false;
    rhysProjectSightHistoryRevealed_=true; vosslerIslandDataCenterKnown_=true;
    return advance(ObjectiveType::Interact,"Kane: Project Sight has a private island data center. Vossler keeps the real operation there.");
}
bool StoryGameplay::escapeWithKaneIntel(){
    if(!rhysProjectSightHistoryRevealed_||!vosslerIslandDataCenterKnown_)return false;
    wanted_.level=4; wanted_.heat=75.0f;
    const bool ok=advance(ObjectiveType::EscapeWanted,"MISSION COMPLETE - Kane",false);
    wanted_.clear();
    if(ok&&mission13Complete()){
        dash_.addMoney(110000); dash_.addXp(9500);
        syncHud("$110,000 + 9,500 XP - Kane captured alive. Vossler's island data center revealed. Mission 14 unlocked: No Safe Place");
    }
    return ok;
}
bool StoryGameplay::mission13Complete()const{return missions_.stateOf("story_13_kane")==MissionState::Completed;}


// Mission 14 - No Safe Place
bool StoryGameplay::startMission14(){
    if(!mission13Complete()||!vosslerIslandDataCenterKnown_) return false;
    if(!missions_.start("story_14_no_safe_place",GameMode::Story)) return false;
    safehouseUnderAttack_=rollingGunfightActive_=policeHelicopterActive_=false; wanted_.clear();
    syncHud("MISSION 14 - NO SAFE PLACE. Mayja: Get back to the safehouse. Vossler knows where we are.");
    return true;
}
bool StoryGameplay::returnToCrewSafehouse(){ safehouseUnderAttack_=true; return advance(ObjectiveType::ReachLocation,"PROJECT SIGHT ASSAULT - Defend the safehouse!"); }
bool StoryGameplay::surviveSafehouseAssault(){ if(!safehouseUnderAttack_)return false; wanted_.level=3; wanted_.heat=60.0f; return advance(ObjectiveType::Interact,"Mayja: We cannot hold this place. Get to the car!"); }
bool StoryGameplay::enterCrewEscapeVehicle(){ if(!safehouseUnderAttack_)return false; rollingGunfightActive_=true; wanted_.level=4; wanted_.heat=80.0f; return advance(ObjectiveType::EnterVehicle,"WANTED LEVEL 4 - Project Sight vehicles and police are closing in."); }
bool StoryGameplay::surviveRollingGunfight(){ if(!rollingGunfightActive_)return false; policeHelicopterActive_=true; wanted_.level=4; wanted_.heat=90.0f; return advance(ObjectiveType::EscapeWanted,"POLICE HELICOPTER IN PURSUIT - Keep moving and break line of sight."); }
bool StoryGameplay::evadePoliceHelicopter(){ if(!policeHelicopterActive_)return false; policeHelicopterActive_=false; rollingGunfightActive_=false; wanted_.clear(); return advance(ObjectiveType::EscapeWanted,"Helicopter lost. Reach the old hideout before Project Sight finds you again."); }
bool StoryGameplay::reachOldHideout(){
    if(policeHelicopterActive_)return false;
    safehouseUnderAttack_=false;
    const bool ok=advance(ObjectiveType::ReachLocation,"MISSION COMPLETE - No Safe Place",false);
    if(ok&&mission14Complete()){ dash_.addMoney(125000); dash_.addXp(11000); syncHud("$125,000 + 11,000 XP - Crew relocated. Mission 15 unlocked: The Hostage"); }
    return ok;
}
bool StoryGameplay::mission14Complete()const{return missions_.stateOf("story_14_no_safe_place")==MissionState::Completed;}


// Mission 15 - The Hostage
bool StoryGameplay::startMission15(){
    if(!mission14Complete()) return false;
    if(!missions_.start("story_15_the_hostage",GameMode::Story)) return false;
    hostageRescued_=hostageProtected_=cleanupRevealed_=false; hotelEntryRoute_.clear(); wanted_.clear();
    syncHud("MISSION 15 - THE HOSTAGE. Mayja: Project Sight has one of ours in the Downtown construction hotel.");
    return true;
}
bool StoryGameplay::reachDowntownConstructionHotel(){ return advance(ObjectiveType::ReachLocation,"Construction hotel located. Find a way inside."); }
bool StoryGameplay::chooseHotelEntryRoute(const std::string& route){
    if(route!="Service Entrance" && route!="Rooftop" && route!="Parking Garage") return false;
    hotelEntryRoute_=route;
    return advance(ObjectiveType::Interact,(std::string("ENTRY ROUTE - ")+route).c_str());
}
bool StoryGameplay::rescueCapturedCrewMember(){
    if(hotelEntryRoute_.empty()) return false; hostageRescued_=true; wanted_.level=3; wanted_.heat=65.0f;
    return advance(ObjectiveType::Interact,"HOSTAGE FOUND - Cut them loose and move!");
}
bool StoryGameplay::protectHostage(){
    if(!hostageRescued_) return false; hostageProtected_=true; wanted_.level=4; wanted_.heat=85.0f;
    return advance(ObjectiveType::Interact,"Mayja: Keep them alive. Parking garage is our exit.");
}
bool StoryGameplay::escapeParkingGarage(){
    if(!hostageProtected_) return false; wanted_.level=4; wanted_.heat=90.0f;
    return advance(ObjectiveType::EscapeWanted,"PARKING GARAGE ESCAPE - Break through Project Sight and lose the pursuit.");
}
bool StoryGameplay::recoverCleanupIntel(){
    if(!hostageProtected_) return false; cleanupRevealed_=true; wanted_.clear();
    const bool ok=advance(ObjectiveType::Interact,"THE CLEANUP - Project Sight is preparing to erase witnesses, evidence and the crew base.",false);
    if(ok&&mission15Complete()){ dash_.addMoney(145000); dash_.addXp(13000); syncHud("$145,000 + 13,000 XP - THE CLEANUP revealed. Mission 16 unlocked: The Cleanup"); }
    return ok;
}
bool StoryGameplay::mission15Complete()const{return missions_.stateOf("story_15_the_hostage")==MissionState::Completed;}


// Mission 16 - The Cleanup
bool StoryGameplay::startMission16(){
    if(!mission15Complete()) return false;
    if(!missions_.start("story_16_the_cleanup",GameMode::Story)) return false;
    cleanupTarget_.clear(); cleanupChoiceLocked_=false; cleanupTargetNeutralized_=false; masterLedgerRecovered_=false; centralReserveRevealed_=false; wanted_.clear();
    syncHud("MISSION 16 - THE CLEANUP. Mayja: They are erasing everything. We can stop one operation before the window closes.");
    return true;
}
bool StoryGameplay::meetMayjaForCleanup(){
    return advance(ObjectiveType::ReachLocation,"THE CLEANUP - Choose: Witnesses, Evidence, Crew Base, or Bank Archive.");
}
bool StoryGameplay::chooseCleanupTarget(const std::string& target){
    if(cleanupChoiceLocked_) return false;
    if(target!="Witnesses" && target!="Evidence" && target!="Crew Base" && target!="Bank Archive") return false;
    cleanupTarget_=target; cleanupChoiceLocked_=true;
    return advance(ObjectiveType::Interact,(std::string("PERMANENT CHOICE - ")+target+". Move now.").c_str());
}
bool StoryGameplay::neutralizeCleanupTarget(){
    if(!cleanupChoiceLocked_) return false;
    cleanupTargetNeutralized_=true; wanted_.level=4; wanted_.heat=82.0f;
    return advance(ObjectiveType::Interact,(std::string("CLEANUP OPERATION STOPPED - ")+cleanupTarget_).c_str());
}
bool StoryGameplay::escapeCleanupRetaliation(){
    if(!cleanupTargetNeutralized_) return false;
    wanted_.level=4; wanted_.heat=90.0f;
    return advance(ObjectiveType::EscapeWanted,"Project Sight retaliation incoming. Break contact and reach Mayja.");
}
bool StoryGameplay::recoverMasterLedger(){
    if(!cleanupTargetNeutralized_) return false;
    wanted_.clear(); masterLedgerRecovered_=true; centralReserveRevealed_=true;
    const bool ok=advance(ObjectiveType::Interact,"MASTER LEDGER RECOVERED - Project Sight's money trail leads to the Sight City Central Reserve.",false);
    if(ok&&mission16Complete()){ dash_.addMoney(175000); dash_.addXp(15000); syncHud("$175,000 + 15,000 XP - Central Reserve revealed. Mission 17 unlocked: The Ledger"); }
    return ok;
}
bool StoryGameplay::mission16Complete()const{return missions_.stateOf("story_16_the_cleanup")==MissionState::Completed;}

// Mission 17 - The Ledger
bool StoryGameplay::startMission17(){
    if(!mission16Complete() || !masterLedgerRecovered_) return false;
    if(!missions_.start("story_17_the_ledger",GameMode::Story)) return false;
    harborPortLedgerDone_=false; airportLedgerDone_=false; southHillsLedgerDone_=false; downtownMoneyTrailDone_=false; ledgerTargetsCompleted_=0; wanted_.clear();
    syncHud("MISSION 17 - THE LEDGER. Mayja: Four money nodes. Hit them in any order and bring me the complete package.");
    return true;
}
bool StoryGameplay::meetMayjaForLedger(){
    return advance(ObjectiveType::ReachLocation,"MASTER LEDGER TARGETS - Harbor / Port, Airport, South Hills, Downtown Money Trail. Complete all four.");
}
bool StoryGameplay::raidHarborPortLedgerNode(){
    if(harborPortLedgerDone_) return false; harborPortLedgerDone_=true; ++ledgerTargetsCompleted_; wanted_.level=3; wanted_.heat=62.0f;
    return advance(ObjectiveType::Interact,"HARBOR / PORT NODE SECURED - Shipping manifests and offshore transfers recovered.");
}
bool StoryGameplay::raidAirportLedgerNode(){
    if(airportLedgerDone_) return false; airportLedgerDone_=true; ++ledgerTargetsCompleted_; wanted_.level=3; wanted_.heat=66.0f;
    return advance(ObjectiveType::Interact,"AIRPORT NODE SECURED - Courier routes and Project Sight flight transfers recovered.");
}
bool StoryGameplay::raidSouthHillsLedgerNode(){
    if(southHillsLedgerDone_) return false; southHillsLedgerDone_=true; ++ledgerTargetsCompleted_; wanted_.level=4; wanted_.heat=74.0f;
    return advance(ObjectiveType::Interact,"SOUTH HILLS NODE SECURED - Shell accounts and private holdings recovered.");
}
bool StoryGameplay::traceDowntownMoneyTrail(){
    if(downtownMoneyTrailDone_) return false; downtownMoneyTrailDone_=true; ++ledgerTargetsCompleted_; wanted_.level=4; wanted_.heat=82.0f;
    return advance(ObjectiveType::Interact,"DOWNTOWN MONEY TRAIL COMPLETE - The flow converges on Sight City Central Reserve.");
}
bool StoryGameplay::deliverCompleteLedgerPackage(){
    if(ledgerTargetsCompleted_!=4) return false; wanted_.clear();
    const bool ok=advance(ObjectiveType::Deliver,"LEDGER COMPLETE - Mayja has the full financial map. Central Reserve is now a viable target.",false);
    if(ok&&mission17Complete()){ dash_.addMoney(210000); dash_.addXp(18000); syncHud("$210,000 + 18,000 XP - Mission complete. Operation Blackout unlocked."); }
    return ok;
}
bool StoryGameplay::mission17Complete()const{return missions_.stateOf("story_17_the_ledger")==MissionState::Completed;}


// Mission 18 - Operation Blackout
bool StoryGameplay::startMission18(){
    if(!mission17Complete()) return false;
    if(!missions_.start("story_18_operation_blackout",GameMode::Story)) return false;
    blackoutOperation_.clear(); blackoutChoiceLocked_=false; blackoutOperationComplete_=false; crewProtectionBonus_=false; blackoutBonusMoney_=0; blackoutBonusXp_=0; wanted_.clear();
    syncHud("MISSION 18 - OPERATION BLACKOUT. Mayja: One strike. Choose what hurts Project Sight most.");
    return true;
}
bool StoryGameplay::meetMayjaForBlackout(){
    return advance(ObjectiveType::ReachLocation,"BLACKOUT OPTIONS - POWER GRID / COMMUNICATIONS / PROTECT THE CREW / HUNT THE COMMANDER.");
}
bool StoryGameplay::chooseBlackoutOperation(const std::string& operation){
    if(blackoutChoiceLocked_) return false;
    if(operation=="POWER GRID"){ blackoutBonusMoney_=60000; blackoutBonusXp_=6000; }
    else if(operation=="COMMUNICATIONS"){ blackoutBonusMoney_=65000; blackoutBonusXp_=6500; }
    else if(operation=="PROTECT THE CREW"){ blackoutBonusMoney_=75000; blackoutBonusXp_=7500; crewProtectionBonus_=true; }
    else if(operation=="HUNT THE COMMANDER"){ blackoutBonusMoney_=100000; blackoutBonusXp_=10000; }
    else return false;
    blackoutOperation_=operation; blackoutChoiceLocked_=true;
    return advance(ObjectiveType::Interact,(std::string("BLACKOUT LOCKED - ")+operation).c_str());
}
bool StoryGameplay::executeBlackoutOperation(){
    if(!blackoutChoiceLocked_ || blackoutOperationComplete_) return false;
    blackoutOperationComplete_=true; wanted_.level=4; wanted_.heat=86.0f;
    return advance(ObjectiveType::Interact,(std::string("BLACKOUT STRIKE COMPLETE - ")+blackoutOperation_+". Project Sight response incoming.").c_str());
}
bool StoryGameplay::escapeBlackoutResponse(){
    if(!blackoutOperationComplete_) return false;
    wanted_.level=4; wanted_.heat=92.0f;
    return advance(ObjectiveType::EscapeWanted,"Break Project Sight response and regroup with Mayja.");
}
bool StoryGameplay::confirmCentralReserveWindow(){
    if(!blackoutOperationComplete_) return false;
    wanted_.clear();
    const bool ok=advance(ObjectiveType::Interact,"BLACKOUT COMPLETE - Central Reserve security window confirmed. The heist is ready.",false);
    if(ok&&mission18Complete()){
        dash_.addMoney(250000+blackoutBonusMoney_); dash_.addXp(20000+blackoutBonusXp_);
        syncHud((std::string("$250,000 + 20,000 XP base, plus Blackout bonus - Mission 19 unlocked: Central Reserve Heist")).c_str());
    }
    return ok;
}
bool StoryGameplay::mission18Complete()const{return missions_.stateOf("story_18_operation_blackout")==MissionState::Completed;}


// Mission 19 - Central Reserve Heist
bool StoryGameplay::startMission19(){
    if(!mission18Complete()) return false;
    if(!missions_.start("story_19_central_reserve_heist",GameMode::Story)) return false;
    centralReserveApproach_.clear(); reserveWeaponsSetup_=reserveGetawaySetup_=reserveAmmoSetup_=false; centralReserveHeistStarted_=false; centralReserveBreached_=false; centralReserveGrossScore_=0; wanted_.clear();
    syncHud("MISSION 19 - CENTRAL RESERVE HEIST. TARGET: $900,000,000."); return true;
}
bool StoryGameplay::meetMayjaForCentralReserve(){ return advance(ObjectiveType::ReachLocation,"Mayja: Three ways in. Quiet Entry, Underground, or Direct Assault. Your call."); }
bool StoryGameplay::chooseCentralReserveApproach(const std::string& a){
    if(!centralReserveApproach_.empty()) return false;
    if(a!="Quiet Entry" && a!="Underground" && a!="Direct Assault") return false;
    centralReserveApproach_=a; return advance(ObjectiveType::Interact,(std::string("APPROACH LOCKED - ")+a).c_str());
}
bool StoryGameplay::completeWeaponsSetup(){ if(centralReserveApproach_.empty()||reserveWeaponsSetup_) return false; reserveWeaponsSetup_=true; dash_.addMoney(40000); dash_.addXp(4000); return advance(ObjectiveType::Interact,"SETUP COMPLETE - Weapons. +$40,000 / +4,000 XP"); }
bool StoryGameplay::completeGetawayVehiclesSetup(){ if(!reserveWeaponsSetup_||reserveGetawaySetup_) return false; reserveGetawaySetup_=true; dash_.addMoney(45000); dash_.addXp(4500); return advance(ObjectiveType::Interact,"SETUP COMPLETE - Getaway Vehicles. +$45,000 / +4,500 XP"); }
bool StoryGameplay::completeAmmoSetup(){ if(!reserveGetawaySetup_||reserveAmmoSetup_) return false; reserveAmmoSetup_=true; dash_.addMoney(35000); dash_.addXp(3500); return advance(ObjectiveType::Interact,"SETUP COMPLETE - Ammo. +$35,000 / +3,500 XP"); }
bool StoryGameplay::confirmCentralReserveHeist(){ if(!centralReserveReady()||centralReserveHeistStarted_) return false; centralReserveHeistStarted_=true; return advance(ObjectiveType::Interact,"HEIST CONFIRMED - Crew, escape routes and backups checked. START HEIST."); }
bool StoryGameplay::breachCentralReserve(){ if(!centralReserveHeistStarted_||centralReserveBreached_) return false; centralReserveBreached_=true; centralReserveGrossScore_=900000000LL; wanted_.level=7; wanted_.heat=100.0f; return advance(ObjectiveType::Interact,"CENTRAL RESERVE BREACHED - $900,000,000 secured. MAXIMUM RESPONSE INBOUND."); }
bool StoryGameplay::escapeCentralReserve(){ if(!centralReserveBreached_) return false; wanted_.level=7; wanted_.heat=100.0f; return advance(ObjectiveType::EscapeWanted,"WANTED LEVEL 7 - Protect the crew vehicles and get the score out of Downtown."); }
bool StoryGameplay::deliverCentralReserveScore(){ if(centralReserveGrossScore_!=900000000LL) return false; wanted_.clear(); const bool ok=advance(ObjectiveType::Deliver,"HEIST COMPLETE - SIGHT CITY CENTRAL RESERVE. TOTAL SCORE: $900,000,000.",false); if(ok&&mission19Complete()){ dash_.addXp(100000); syncHud("CENTRAL RESERVE HEIST COMPLETE - +100,000 XP. Crew cuts and aftermath unlocked."); } return ok; }
bool StoryGameplay::mission19Complete()const{return missions_.stateOf("story_19_central_reserve_heist")==MissionState::Completed;}

// M55 - Central Reserve aftermath and crew cuts
bool StoryGameplay::startCentralReserveAftermath(){
    if(!mission19Complete()) return false;
    if(!missions_.start("story_20_central_reserve_aftermath",GameMode::Story)) return false;
    postHeistResponseSurvived_=false; centralReserveMoneySecured_=false; centralReserveCutsAllocated_=false;
    vosslerAftermathTauntReceived_=false; dashCentralReserveCut_=mayjaCentralReserveCut_=rhysCentralReserveCut_=crewReserveCut_=0;
    wanted_.level=7; wanted_.heat=100.0f;
    syncHud("CENTRAL RESERVE AFTERMATH - Project Sight and police are hunting the crew and the $900,000,000 score.");
    return true;
}
bool StoryGameplay::survivePostHeistResponse(){
    wanted_.level=7; wanted_.heat=100.0f;
    const bool ok=advance(ObjectiveType::EscapeWanted,"AFTERMATH - Break the combined police and Project Sight dragnet.");
    if(ok) postHeistResponseSurvived_=true;
    return ok;
}
bool StoryGameplay::secureCentralReserveMoney(){
    if(!postHeistResponseSurvived_ || centralReserveGrossScore_!=900000000LL) return false;
    const bool ok=advance(ObjectiveType::Deliver,"SCORE SECURED - All surviving money vehicles reached the crew's hidden transfer point.");
    if(ok) centralReserveMoneySecured_=true;
    return ok;
}
bool StoryGameplay::allocateCentralReserveCrewCuts(){
    if(!centralReserveMoneySecured_ || centralReserveCutsAllocated_) return false;
    dashCentralReserveCut_=360000000LL; mayjaCentralReserveCut_=225000000LL; rhysCentralReserveCut_=180000000LL; crewReserveCut_=135000000LL;
    centralReserveCutsAllocated_=true; dash_.addMoney((int)dashCentralReserveCut_);
    return advance(ObjectiveType::Interact,"CREW CUTS COMPLETE - Dash $360M / Mayja $225M / Rhys $180M / Crew Reserve $135M.");
}
bool StoryGameplay::receiveVosslerAftermathTaunt(){
    if(!centralReserveCutsAllocated_) return false;
    const bool ok=advance(ObjectiveType::Interact,"Vossler: You stole money. I still own the city, the response, and everyone hunting you.");
    if(ok) vosslerAftermathTauntReceived_=true;
    return ok;
}
bool StoryGameplay::regroupAfterCentralReserve(){
    if(!vosslerAftermathTauntReceived_) return false;
    wanted_.clear();
    const bool ok=advance(ObjectiveType::ReachLocation,"AFTERMATH COMPLETE - Crew, surviving vehicles and the score are accounted for. Prepare for the final war.",false);
    if(ok&&centralReserveAftermathComplete()){ dash_.addMoney(250000); dash_.addXp(25000); syncHud("+$250,000 + 25,000 XP - Final battle preparation unlocked."); }
    return ok;
}
bool StoryGameplay::centralReserveAftermathComplete()const{return missions_.stateOf("story_20_central_reserve_aftermath")==MissionState::Completed;}


// M56 - final battle preparation before TAKE SIGHT CITY
bool StoryGameplay::startFinalBattlePreparation(){
    if(!centralReserveAftermathComplete()) return false;
    if(!missions_.start("story_21_final_battle_preparation",GameMode::Story)) return false;
    finalAmmo_=finalWeapons_=finalCars_=finalArmor_=finalHeavy_=false;
    finalSuppliesBoardReviewed_=false; takeSightCityUnlocked_=false; finalSupplyCount_=0;
    return advance(ObjectiveType::ReachLocation,"FINAL WAR - Meet Mayja at the staging area. Build the supplies board for TAKE SIGHT CITY.");
}
bool StoryGameplay::gatherFinalAmmo(){ if(finalAmmo_)return false; finalAmmo_=true; ++finalSupplyCount_; return advance(ObjectiveType::Interact,"SUPPLIES: AMMO secured and added to the physical staging board."); }
bool StoryGameplay::gatherFinalWeapons(){ if(!finalAmmo_||finalWeapons_)return false; finalWeapons_=true; ++finalSupplyCount_; return advance(ObjectiveType::Interact,"SUPPLIES: WEAPONS secured and added to the staging board."); }
bool StoryGameplay::gatherFinalCars(){ if(!finalWeapons_||finalCars_)return false; finalCars_=true; ++finalSupplyCount_; return advance(ObjectiveType::Interact,"SUPPLIES: CARS secured. Getaway and assault vehicles are staged."); }
bool StoryGameplay::gatherFinalArmor(){ if(!finalCars_||finalArmor_)return false; finalArmor_=true; ++finalSupplyCount_; return advance(ObjectiveType::Interact,"SUPPLIES: ARMOR secured for Dash and the crew."); }
bool StoryGameplay::gatherFinalHeavyResources(){ if(!finalArmor_||finalHeavy_)return false; finalHeavy_=true; ++finalSupplyCount_; return advance(ObjectiveType::Interact,"SUPPLIES: HEAVY resources secured. Final stockpile complete."); }
bool StoryGameplay::reviewFinalSuppliesBoard(){
    if(!finalSuppliesReady()||finalSuppliesBoardReviewed_) return false;
    finalSuppliesBoardReviewed_=true;
    return advance(ObjectiveType::Interact,"SUPPLIES BOARD: AMMO / WEAPONS / CARS / ARMOR / HEAVY - READY. Choose crew and loadout.");
}
bool StoryGameplay::confirmTakeSightCityReady(bool ready){
    if(!finalSuppliesBoardReviewed_||!ready) { syncHud("NOT YET - Get more supplies, change loadout or choose crew before the final assault."); return false; }
    const bool ok=advance(ObjectiveType::Interact,"I'M READY - TAKE SIGHT CITY unlocked. The outer territories come first; Downtown is last.",false);
    if(ok&&finalBattlePreparationComplete()){ takeSightCityUnlocked_=true; dash_.addMoney(500000); dash_.addXp(50000); syncHud("+$500,000 + 50,000 XP - TAKE SIGHT CITY UNLOCKED"); }
    return ok;
}
bool StoryGameplay::finalBattlePreparationComplete()const{return missions_.stateOf("story_21_final_battle_preparation")==MissionState::Completed;}


// M57 - TAKE SIGHT CITY
bool StoryGameplay::startTakeSightCity(){
    if(!takeSightCityUnlocked_ || !finalBattlePreparationComplete()) return false;
    if(!missions_.start("story_22_take_sight_city",GameMode::Story)) return false;
    finalTerritoriesCaptured_=0; finalCapturedTerritories_.clear(); downtownFinalUnlocked_=false;
    downtownBreached_=false; vosslerForcesDefeated_=false; vosslerChaseActive_=false; vosslerDefeated_=false; storyComplete_=false;
    return advance(ObjectiveType::ReachLocation,"TAKE SIGHT CITY - Choose an outer territory. Downtown stays locked until Vossler's outer defenses fall.");
}
bool StoryGameplay::captureFinalTerritory(const std::string& territory){
    static const char* allowed[]={"Harbor","Industrial Zone","Airport","West Suburbs","East Suburbs","North Countryside","West Coast","East Coast","South Hills","Desert Plains","Port"};
    bool valid=false; for(const char* a:allowed) if(territory==a){valid=true;break;}
    if(!valid || downtownBreached_ || std::find(finalCapturedTerritories_.begin(),finalCapturedTerritories_.end(),territory)!=finalCapturedTerritories_.end()) return false;
    finalCapturedTerritories_.push_back(territory); ++finalTerritoriesCaptured_;
    const bool ok=advance(ObjectiveType::Interact,("TERRITORY CAPTURED - "+territory+". Crew holds the district.").c_str());
    if(ok && finalTerritoriesCaptured_>=5){ downtownFinalUnlocked_=true; syncHud("OUTER DEFENSES BROKEN - DOWNTOWN UNLOCKED"); }
    return ok;
}
bool StoryGameplay::breachDowntown(){
    if(!downtownFinalUnlocked_) return false;
    const bool ok=advance(ObjectiveType::ReachLocation,"DOWNTOWN - Final assault. Push through Project Sight and reach Vossler.");
    if(ok){ downtownBreached_=true; wanted_.level=7; wanted_.heat=100.0f; } return ok;
}
bool StoryGameplay::defeatVosslerForces(){ if(!downtownBreached_)return false; const bool ok=advance(ObjectiveType::Interact,"DOWNTOWN SECURED - Vossler is escaping. Dash: This ends now."); if(ok)vosslerForcesDefeated_=true; return ok; }
bool StoryGameplay::chaseVossler(){ if(!vosslerForcesDefeated_)return false; const bool ok=advance(ObjectiveType::ReachLocation,"CHASE VOSSLER - Stay on him. Do not let him leave Sight City."); if(ok)vosslerChaseActive_=true; return ok; }
bool StoryGameplay::defeatAdrianVossler(){ if(!vosslerChaseActive_)return false; const bool ok=advance(ObjectiveType::Interact,"ADRIAN VOSSLER DEFEATED - Project Sight's founder is dead. His control of Sight City ends here."); if(ok){vosslerDefeated_=true; wanted_.clear();} return ok; }
bool StoryGameplay::finishTakeSightCity(){ if(!vosslerDefeated_)return false; const bool ok=advance(ObjectiveType::Interact,"STORY COMPLETE - 100% CONTROL. Credits will roll, then Free Roam begins.",false); if(ok&&takeSightCityComplete()){storyComplete_=true; dash_.addMoney(1000000); dash_.addXp(100000); syncHud("SIGHT CITY: UNDERWORLD - STORY COMPLETE +$1,000,000 +100,000 XP");} return ok; }
bool StoryGameplay::takeSightCityComplete()const{return missions_.stateOf("story_22_take_sight_city")==MissionState::Completed;}


// M58 - Story epilogue and persistent post-story Free Roam
bool StoryGameplay::startCredits(){
    if(!storyComplete_ || !vosslerDefeated_ || creditsStarted_) return false;
    creditsStarted_=true;
    syncHud("CREDITS - SIGHT CITY: UNDERWORLD");
    return true;
}
bool StoryGameplay::finishCredits(){
    if(!creditsStarted_ || creditsFinished_) return false;
    creditsFinished_=true;
    syncHud("CREDITS COMPLETE");
    return true;
}
bool StoryGameplay::beginFewWeeksLater(){
    if(!creditsFinished_ || fewWeeksLaterShown_) return false;
    fewWeeksLaterShown_=true;
    wanted_.clear();
    syncHud("A FEW WEEKS LATER - Sight City is moving again.");
    return true;
}
bool StoryGameplay::enterPostStoryFreeRoam(){
    if(!fewWeeksLaterShown_ || !storyComplete_ || !vosslerDefeated_) return false;
    postStoryFreeRoam_=true;
    wanted_.clear();
    syncHud("FREE ROAM - Dynamic gangs and territory attacks restored. Crew contacts and Mayja's smaller heists are available.");
    return true;
}
