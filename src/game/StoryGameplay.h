#pragma once
#include "MissionSystem.h"
#include "CharacterProfile.h"
#include "Player.h"
#include "Vehicle.h"
#include "TimeOfDaySystem.h"
#include "WeatherSystem.h"
#include "WantedSystem.h"
#include "EmergencyResponseSystem.h"
#include "DynamicIncidentSystem.h"
#include "CharacterMeetingSystem.h"
#include "Settings.h"
#include "StoryHeistSystem.h"
#include <string>

struct StoryHudState {
    std::string missionTitle;
    std::string objectiveText;
    std::string notification;
    int money=0;
    int xp=0;
    int checkpoint=0;
    int chasePressure=0;
};

class StoryGameplay {
public:
    StoryGameplay();
    // Mission 1 - Back in Sight City
    bool startMission1();
    bool reachRhys();
    bool enterRhysVehicle();
    bool reachDowntown();
    bool reachWestSuburbs();
    bool reachEastCoast();
    bool reachNorthCountryside();
    bool reachMountRidge();
    bool reachNightCity();
    bool completeTour();
    bool mission1Complete() const;

    // Mission 2 - Like Old Times
    bool startMission2();
    bool reachOldGarage();
    bool enterDeliveryCar();
    bool deliverCar();
    bool gangChaseStarts();
    bool loseRivalGang();
    bool reachOldWorkshop();
    bool completeMission2();
    bool mission2Complete() const;

    // Mission 3 - Easy Money
    bool startMission3();
    bool reachHarborDocks();
    bool locateElectronicsContainer();
    bool enterContainerTruck();
    bool loseHarborRivals();
    bool deliverElectronics();
    bool mission3Complete() const;

    // Mission 4 - The Long Way Home
    bool startMission4();
    bool escapeIndustrialZone();
    bool reachHighway();
    bool enterMountainTunnels();
    bool survivePolicePursuit();
    bool reachNorthCountrysideGarage();
    bool mission4Complete() const;

    // Mission 5 - Our First Score
    bool startMission5();
    bool meetMayjaAtHeistRoom();
    bool chooseGetawayVehicle(const std::string& vehicleId,bool owned=true,bool heistCompatible=true,bool missionLocked=false);
    bool collectHeistEquipment();
    bool chooseHeistEscapeRoute(EscapeRoute route);
    bool reachArmoredTransport();
    bool takeArmoredTransportScore();
    bool enterHeistGetawayVehicle();
    bool escapeFirstHeist();
    bool deliverFirstScore();
    bool mission5Complete() const;

    // Mission 6 - Something's Off
    bool startMission6();
    bool meetMayjaAfterFirstScore();
    bool reviewNextJobWithoutRhys();
    bool findRhysNearRivalDistrict();
    bool observeRhysFromDistance();
    bool followRhysOptional();
    bool reportSuspicionToMayja();
    bool mission6Complete() const;

    // Mission 7 - Wrong Place, Wrong Time
    bool startMission7();
    bool meetRhysForIndustrialJob();
    bool enterIndustrialJobSite();
    bool discoverRivalAmbush();
    bool surviveRivalAmbush();
    bool rhysSavesDash();
    bool escapeIndustrialAmbush();
    bool debriefMayjaAfterAmbush();
    bool mission7Complete() const;
    bool rivalAmbushActive() const { return rivalAmbushActive_; }
    bool rhysRescueTriggered() const { return rhysRescueTriggered_; }
    bool insiderConfirmed() const { return insiderConfirmed_; }

    // Mission 8 - Behind My Back
    bool startMission8();
    bool meetMayjaForSurveillance();
    bool tailRhysUndetected();
    bool captureRhysMeetingEvidence();
    bool leaveRivalMeetingArea();
    bool showRhysEvidenceToMayja();
    bool mission8Complete() const;
    bool rhysMeetingConfirmed() const { return rhysMeetingConfirmed_; }
    bool betrayalEvidenceSecured() const { return betrayalEvidenceSecured_; }

    // Mission 9 - Betrayed
    bool startMission9();
    bool briefFakeHeistTrap();
    bool plantFakeHeistInformation();
    bool observeRivalsTakeTheBait();
    bool confrontRhys();
    bool hearRhysConfession();
    bool learnAboutBiggerPlayer();
    bool mission9Complete() const;
    bool rhysBetrayalConfirmed() const { return rhysBetrayalConfirmed_; }
    bool biggerPlayerRevealed() const { return biggerPlayerRevealed_; }

    // Mission 10 - Follow the Money
    bool startMission10(); bool traceIndustrialPayment(); bool recoverPaymentRecord();
    bool followCourierDowntown(); bool trackTransferToAirport(); bool followMoneyToWestCoast();
    bool identifyProjectSight(); bool mission10Complete() const;
    bool vosslerHoldingsIdentified() const { return vosslerHoldingsIdentified_; }
    bool projectSightDiscovered() const { return projectSightDiscovered_; }

    // Mission 11 - The Facility
    bool startMission11();
    bool reachHiddenFacility();
    bool infiltrateFacility();
    bool reachProjectSightOperationsRoom();
    bool downloadProjectSightTargetMaps();
    bool recoverCrewDossiers();
    bool identifyAdrianVossler();
    bool escapeFacilityWithArchive();
    bool mission11Complete() const;
    bool facilityInfiltrated() const { return facilityInfiltrated_; }
    bool projectSightArchiveSecured() const { return projectSightArchiveSecured_; }
    bool adrianVosslerNamed() const { return adrianVosslerNamed_; }

    // Mission 12 - Phase II
    bool startMission12();
    bool reviewPhaseIICountdown();
    bool disruptPowerGrid();
    bool disruptTransportNetwork();
    bool disruptMoneyNetwork();
    bool returnToDowntownLockdown();
    bool identifyEliasKane();
    bool surviveVosslerLockdown();
    bool mission12Complete() const;
    int phaseIITargetsCompleted() const { return phaseIITargetsCompleted_; }
    bool eliasKaneIdentified() const { return eliasKaneIdentified_; }
    bool phaseIILockdownActive() const { return phaseIILockdownActive_; }

    // Mission 13 - Kane
    bool startMission13();
    bool trackEliasKane();
    bool reachKaneSecurityConvoy();
    bool disableKaneEscort();
    bool captureKaneAlive();
    bool interrogateKane();
    bool learnRhysProjectSightHistory();
    bool escapeWithKaneIntel();
    bool mission13Complete() const;
    bool kaneCapturedAlive() const { return kaneCapturedAlive_; }
    bool rhysProjectSightHistoryRevealed() const { return rhysProjectSightHistoryRevealed_; }
    bool vosslerIslandDataCenterKnown() const { return vosslerIslandDataCenterKnown_; }

    // Mission 14 - No Safe Place
    bool startMission14();
    bool returnToCrewSafehouse();
    bool surviveSafehouseAssault();
    bool enterCrewEscapeVehicle();
    bool surviveRollingGunfight();
    bool evadePoliceHelicopter();
    bool reachOldHideout();
    bool mission14Complete() const;
    bool safehouseUnderAttack() const { return safehouseUnderAttack_; }
    bool policeHelicopterActive() const { return policeHelicopterActive_; }

    // Mission 15 - The Hostage
    bool startMission15();
    bool reachDowntownConstructionHotel();
    bool chooseHotelEntryRoute(const std::string& route);
    bool rescueCapturedCrewMember();
    bool protectHostage();
    bool escapeParkingGarage();
    bool recoverCleanupIntel();
    bool mission15Complete() const;
    bool hostageRescued() const { return hostageRescued_; }
    bool cleanupRevealed() const { return cleanupRevealed_; }
    const std::string& hotelEntryRoute() const { return hotelEntryRoute_; }

    // Mission 16 - The Cleanup
    bool startMission16();
    bool meetMayjaForCleanup();
    bool chooseCleanupTarget(const std::string& target);
    bool neutralizeCleanupTarget();
    bool escapeCleanupRetaliation();
    bool recoverMasterLedger();
    bool mission16Complete() const;
    const std::string& cleanupTarget() const { return cleanupTarget_; }
    bool cleanupChoiceLocked() const { return cleanupChoiceLocked_; }
    bool masterLedgerRecovered() const { return masterLedgerRecovered_; }
    bool centralReserveRevealed() const { return centralReserveRevealed_; }

    // Mission 17 - The Ledger
    bool startMission17();
    bool meetMayjaForLedger();
    bool raidHarborPortLedgerNode();
    bool raidAirportLedgerNode();
    bool raidSouthHillsLedgerNode();
    bool traceDowntownMoneyTrail();
    bool deliverCompleteLedgerPackage();
    bool mission17Complete() const;
    int ledgerTargetsCompleted() const { return ledgerTargetsCompleted_; }
    bool harborPortLedgerDone() const { return harborPortLedgerDone_; }
    bool airportLedgerDone() const { return airportLedgerDone_; }
    bool southHillsLedgerDone() const { return southHillsLedgerDone_; }
    bool downtownMoneyTrailDone() const { return downtownMoneyTrailDone_; }

    // Mission 18 - Operation Blackout
    bool startMission18();
    bool meetMayjaForBlackout();
    bool chooseBlackoutOperation(const std::string& operation);
    bool executeBlackoutOperation();
    bool escapeBlackoutResponse();
    bool confirmCentralReserveWindow();
    bool mission18Complete() const;
    const std::string& blackoutOperation() const { return blackoutOperation_; }
    bool blackoutChoiceLocked() const { return blackoutChoiceLocked_; }
    bool blackoutOperationComplete() const { return blackoutOperationComplete_; }
    int blackoutBonusMoney() const { return blackoutBonusMoney_; }
    int blackoutBonusXp() const { return blackoutBonusXp_; }
    bool crewProtectionBonus() const { return crewProtectionBonus_; }

    // Mission 19 - Central Reserve Heist
    bool startMission19();
    bool meetMayjaForCentralReserve();
    bool chooseCentralReserveApproach(const std::string& approach);
    bool completeWeaponsSetup();
    bool completeGetawayVehiclesSetup();
    bool completeAmmoSetup();
    bool confirmCentralReserveHeist();
    bool breachCentralReserve();
    bool escapeCentralReserve();
    bool deliverCentralReserveScore();
    bool mission19Complete() const;
    const std::string& centralReserveApproach() const { return centralReserveApproach_; }
    bool centralReserveReady() const { return reserveWeaponsSetup_ && reserveGetawaySetup_ && reserveAmmoSetup_; }
    long long centralReserveGrossScore() const { return centralReserveGrossScore_; }
    bool centralReserveHeistStarted() const { return centralReserveHeistStarted_; }

    // M55 - Central Reserve aftermath and crew cuts
    bool startCentralReserveAftermath();
    bool survivePostHeistResponse();
    bool secureCentralReserveMoney();
    bool allocateCentralReserveCrewCuts();
    bool receiveVosslerAftermathTaunt();
    bool regroupAfterCentralReserve();
    bool centralReserveAftermathComplete() const;
    bool centralReserveCutsAllocated() const { return centralReserveCutsAllocated_; }
    long long dashCentralReserveCut() const { return dashCentralReserveCut_; }
    long long mayjaCentralReserveCut() const { return mayjaCentralReserveCut_; }
    long long rhysCentralReserveCut() const { return rhysCentralReserveCut_; }
    long long crewReserveCut() const { return crewReserveCut_; }
    bool vosslerAftermathTauntReceived() const { return vosslerAftermathTauntReceived_; }

    // M56 - final battle preparation
    bool startFinalBattlePreparation();
    bool gatherFinalAmmo();
    bool gatherFinalWeapons();
    bool gatherFinalCars();
    bool gatherFinalArmor();
    bool gatherFinalHeavyResources();
    bool reviewFinalSuppliesBoard();
    bool confirmTakeSightCityReady(bool ready);
    bool finalBattlePreparationComplete() const;
    int finalSupplyCount() const { return finalSupplyCount_; }
    bool finalSuppliesReady() const { return finalAmmo_ && finalWeapons_ && finalCars_ && finalArmor_ && finalHeavy_; }
    bool takeSightCityUnlocked() const { return takeSightCityUnlocked_; }

    // M57 - TAKE SIGHT CITY final assault
    bool startTakeSightCity();
    bool captureFinalTerritory(const std::string& territory);
    bool breachDowntown();
    bool defeatVosslerForces();
    bool chaseVossler();
    bool defeatAdrianVossler();
    bool finishTakeSightCity();
    bool takeSightCityComplete() const;
    int finalTerritoriesCaptured() const { return finalTerritoriesCaptured_; }
    bool downtownFinalUnlocked() const { return downtownFinalUnlocked_; }
    bool vosslerDefeated() const { return vosslerDefeated_; }
    bool storyComplete() const { return storyComplete_; }

    // M58 - Credits / A Few Weeks Later / post-story Free Roam
    bool startCredits();
    bool finishCredits();
    bool beginFewWeeksLater();
    bool enterPostStoryFreeRoam();
    bool creditsStarted() const { return creditsStarted_; }
    bool creditsFinished() const { return creditsFinished_; }
    bool fewWeeksLaterShown() const { return fewWeeksLaterShown_; }
    bool postStoryFreeRoam() const { return postStoryFreeRoam_; }
    bool vosslerPermanentlyGone() const { return vosslerDefeated_ && storyComplete_; }
    bool dynamicGangActivityEnabled() const { return postStoryFreeRoam_; }
    bool territoryAttacksEnabled() const { return postStoryFreeRoam_; }
    bool survivingCrewCallable() const { return postStoryFreeRoam_; }
    bool mayjaSmallHeistsEnabled() const { return postStoryFreeRoam_; }

    bool rhysFollowed() const { return rhysFollowed_; }
    bool mayjaLeakDetected() const { return mayjaLeakDetected_; }
    const StoryHeistPlan& heistPlan() const { return storyHeist_.plan(); }
    DynamicIncident spawnDynamicIncident(DynamicIncidentType type);
    MeetingMarker characterMeeting(const std::string& character) const;
    GameSettings& settings(){ return settings_; }
    bool gangChaseActive() const { return gangChaseActive_; }

    bool failAndRetry();
    void update(float dt);
    const StoryHudState& hud() const { return hud_; }
    const MissionSystem& missions() const { return missions_; }
    const CharacterProfile& dash() const { return dash_; }
private:
    bool advance(ObjectiveType type,const char* notification,bool checkpoint=true);
    void syncHud(const char* notification=nullptr);
    MissionSystem missions_;
    CharacterProfile dash_;
    Player player_;
    Vehicle rhysCar_;
    Vehicle deliveryCar_;
    Vehicle containerTruck_;
    Vehicle escapeCar_;
    TimeOfDaySystem time_;
    WeatherSystem weather_;
    WantedSystem wanted_;
    EmergencyResponseSystem emergency_;
    DynamicIncidentSystem incidents_;
    CharacterMeetingSystem meetings_;
    GameSettings settings_;
    StoryHeistSystem storyHeist_;
    Vehicle heistGetawayCar_;
    StoryHudState hud_;
    bool gangChaseActive_{false};
    bool rhysFollowed_{false};
    bool mayjaLeakDetected_{false};
    bool rivalAmbushActive_{false};
    bool rhysRescueTriggered_{false};
    bool insiderConfirmed_{false};
    bool rhysMeetingConfirmed_{false};
    bool betrayalEvidenceSecured_{false};
    bool fakeHeistTrapActive_{false};
    bool rhysBetrayalConfirmed_{false};
    bool biggerPlayerRevealed_{false};
    bool vosslerHoldingsIdentified_{false};
    bool projectSightDiscovered_{false};
    bool facilityInfiltrated_{false};
    bool projectSightArchiveSecured_{false};
    bool crewDossiersRecovered_{false};
    bool adrianVosslerNamed_{false};
    bool phaseIICountdownReviewed_{false};
    bool phaseIIPowerDone_{false};
    bool phaseIITransportDone_{false};
    bool phaseIIMoneyDone_{false};
    bool phaseIILockdownActive_{false};
    bool eliasKaneIdentified_{false};
    int phaseIITargetsCompleted_{0};
    bool kaneTracked_{false};
    bool kaneEscortDisabled_{false};
    bool kaneCapturedAlive_{false};
    bool kaneInterrogated_{false};
    bool rhysProjectSightHistoryRevealed_{false};
    bool vosslerIslandDataCenterKnown_{false};
    bool safehouseUnderAttack_{false};
    bool rollingGunfightActive_{false};
    bool policeHelicopterActive_{false};
    bool hostageRescued_{false};
    bool hostageProtected_{false};
    bool cleanupRevealed_{false};
    std::string hotelEntryRoute_;
    std::string cleanupTarget_;
    bool cleanupChoiceLocked_{false};
    bool cleanupTargetNeutralized_{false};
    bool masterLedgerRecovered_{false};
    bool centralReserveRevealed_{false};
    bool harborPortLedgerDone_{false};
    bool airportLedgerDone_{false};
    bool southHillsLedgerDone_{false};
    bool downtownMoneyTrailDone_{false};
    int ledgerTargetsCompleted_{0};
    std::string blackoutOperation_;
    bool blackoutChoiceLocked_{false};
    bool blackoutOperationComplete_{false};
    bool crewProtectionBonus_{false};
    int blackoutBonusMoney_{0};
    int blackoutBonusXp_{0};
    std::string centralReserveApproach_;
    bool reserveWeaponsSetup_{false};
    bool reserveGetawaySetup_{false};
    bool reserveAmmoSetup_{false};
    bool centralReserveHeistStarted_{false};
    bool centralReserveBreached_{false};
    long long centralReserveGrossScore_{0};
    bool postHeistResponseSurvived_{false};
    bool centralReserveMoneySecured_{false};
    bool centralReserveCutsAllocated_{false};
    bool vosslerAftermathTauntReceived_{false};
    long long dashCentralReserveCut_{0};
    long long mayjaCentralReserveCut_{0};
    long long rhysCentralReserveCut_{0};
    long long crewReserveCut_{0};
    bool finalAmmo_{false};
    bool finalWeapons_{false};
    bool finalCars_{false};
    bool finalArmor_{false};
    bool finalHeavy_{false};
    bool finalSuppliesBoardReviewed_{false};
    bool takeSightCityUnlocked_{false};
    int finalSupplyCount_{0};
    int finalTerritoriesCaptured_{0};
    bool downtownFinalUnlocked_{false};
    bool downtownBreached_{false};
    bool vosslerForcesDefeated_{false};
    bool vosslerChaseActive_{false};
    bool vosslerDefeated_{false};
    bool storyComplete_{false};
    bool creditsStarted_{false};
    bool creditsFinished_{false};
    bool fewWeeksLaterShown_{false};
    bool postStoryFreeRoam_{false};
    std::vector<std::string> finalCapturedTerritories_;
    float gangChasePressure_{0.0f};
};
