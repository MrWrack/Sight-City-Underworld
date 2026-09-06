#pragma once
#include "core/Math.h"
#include "game/NPC.h"
#include "game/FireExplosionSystem.h"
#include "game/Traffic.h"
#include <vector>
#include <cstdint>

// M32: unified police / ambulance / fire response for Story Mode.
enum class EmergencyService:uint8_t { Police, Ambulance, FireDepartment };
enum class EmergencyIncidentType:uint8_t { MinorCrash, MajorCrash, Fire, Explosion, Shooting, MedicalEmergency, Crime };
enum class EmergencyUnitState:uint8_t {
    Available, Dispatched, EnRoute, SecuringScene, Treating, Suppressing,
    Transporting, Pursuing, Returning
};

struct Casualty {
    int id{0};
    Vec3 position{};
    float injurySeverity{0};       // 0..1
    bool stabilized{false};
    bool transported{false};
    bool alive{true};
};

struct EmergencyIncident {
    int id{0};
    EmergencyIncidentType type{EmergencyIncidentType::MinorCrash};
    Vec3 position{};
    float severity{0};             // 0..1
    bool crimeSuspected{false};
    bool firePresent{false};
    bool active{true};
    int policeAssigned{0};
    int ambulanceAssigned{0};
    int fireAssigned{0};
};

struct EmergencyUnit {
    int id{0};
    EmergencyService service{EmergencyService::Police};
    Vec3 position{};
    Vec3 home{};
    Vec3 target{};
    EmergencyUnitState state{EmergencyUnitState::Available};
    int incidentId{-1};
    float responseTimer{0};
    float resource{100};            // water / medical supplies / police readiness
    bool siren{false};
    bool emergencyLights{false};
    bool carryingPatient{false};
};

class EmergencyResponseSystem {
public:
    explicit EmergencyResponseSystem(bool vitaBudget=false);

    int reportIncident(EmergencyIncidentType type,const Vec3& p,float severity,
                       bool crimeSuspected=false,bool firePresent=false);
    void reportFire(const Vec3& p,float severity); // M31 compatibility
    int reportCasualty(const Vec3& p,float injurySeverity);

    void update(float dt,FireExplosionSystem& fire);
    void reactPedestrians(NPCSystem& npcs,const FireExplosionSystem& fire,float dangerRadius=18.f) const;
    void applyTrafficYield(TrafficSystem& traffic,float radius=20.f);

    int activeResponses() const;
    int activeUnits(EmergencyService service) const;
    int suppressedFires() const { return suppressed_; }
    int stabilizedPatients() const { return stabilized_; }
    int transportedPatients() const { return transported_; }
    int securedScenes() const { return securedScenes_; }
    int trafficYieldEvents() const { return trafficYieldEvents_; }
    const std::vector<EmergencyUnit>& units() const { return units_; }
    const std::vector<EmergencyIncident>& incidents() const { return incidents_; }
    const std::vector<Casualty>& casualties() const { return casualties_; }

private:
    std::vector<EmergencyUnit> units_;
    std::vector<EmergencyIncident> incidents_;
    std::vector<Casualty> casualties_;
    int nextIncidentId_{1};
    int nextCasualtyId_{1};
    int suppressed_{0};
    int stabilized_{0};
    int transported_{0};
    int securedScenes_{0};
    int trafficYieldEvents_{0};

    void dispatchNeededUnits(EmergencyIncident& incident);
    bool dispatchOne(EmergencyService service,EmergencyIncident& incident);
    EmergencyIncident* findIncident(int id);
    Casualty* findNearestUntreatedCasualty(const Vec3& p,float radius);
    bool incidentStillNeedsService(const EmergencyIncident& incident,EmergencyService service) const;
    void updateUnit(EmergencyUnit& unit,float dt,FireExplosionSystem& fire);
    void returnUnit(EmergencyUnit& unit);
};
