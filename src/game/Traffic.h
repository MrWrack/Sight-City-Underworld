#pragma once
#include "core/Math.h"
#include "game/Vehicle.h"
#include "game/World.h"
#include "game/RoadNetwork.h"
#include "game/VehiclePhysics.h"
#include <vector>
#include <string>
#include <cstdint>

enum class TrafficVehicleType:uint8_t { Car, SportsCar, LuxuryCar, Taxi, Bus, Truck, Motorcycle, Police, Ambulance, Utility, OffRoad };
enum class TrafficSignal:uint8_t { Green, Yellow, Red };
enum class DriverTemperament:uint8_t { Calm, Normal, Impatient, Aggressive };
enum class TrafficAIState:uint8_t { Cruising, Following, StoppedAtLight, Avoiding, Overtaking, Parking, Parked, FleeingPolice, PolicePursuit };
struct TrafficLight { Vec3 position; float phase{0}; TrafficSignal northSouth{TrafficSignal::Green}; TrafficSignal eastWest{TrafficSignal::Red}; };
struct TrafficCar {
    Vec3 position; float heading{0}; float speed{0}; float cruiseSpeed{0};
    TrafficVehicleType type{TrafficVehicleType::Car}; VehicleLighting lighting{};
    int lane{0}; float decisionTimer{0};
    std::string model{"Vivo City"}; DriverTemperament temperament{DriverTemperament::Normal}; TrafficAIState aiState{TrafficAIState::Cruising};
    Vec3 destination{}; float laneOffset{0}; float stateTimer{0}; bool policeTarget{false}; bool parked{false};
    RoadRoute route{}; int id{0}; int chaseTarget{-1}; float collisionRadius{2.1f};
    VehiclePhysicsBody physics{};
};
class TrafficSystem {
public:
    std::vector<TrafficCar> cars; std::vector<TrafficLight> lights;
    void update(const Vec3& player,float dt,RegionType region=RegionType::Downtown);
    void setPoliceAlert(bool active){ policeAlert=active; }
    int activeDrivers() const { return (int)cars.size(); }
    int fleeingDrivers() const;
    int policePursuits() const;
    int avoidedCollisions() const { return collisionAvoidanceEvents; }
    int physicalCollisions() const { return physicalCollisionEvents; }
    std::size_t roadNodeCount() const { return roads.nodeCount(); }
private:
    bool policeAlert{false};
    RoadNetwork roads;
    Vec3 roadCenter{};
    RegionType roadRegion{RegionType::Downtown};
    int collisionAvoidanceEvents{0};
    int physicalCollisionEvents{0};
    VehiclePhysicsSystem vehiclePhysics;
    void ensurePopulation(const Vec3& player,RegionType region);
    void updateLights(float dt);
    void updateDriver(size_t i,const Vec3& player,float dt,RegionType region);
    void refreshRoute(TrafficCar& c,const Vec3& player);
    void assignPoliceChases();
};
