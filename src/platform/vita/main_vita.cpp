#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/Camera.h"
#include "game/World.h"
#include "game/EnvironmentSystem.h"
#include "game/SightCityMap.h"
#include "game/WorldCollisionSystem.h"
#include "game/WantedSystem.h"
#include "game/Settings.h"
#include "game/Traffic.h"
#include "game/NPC.h"
#include "platform/vita/VitaInput.h"
#include "platform/vita/VitaRenderer.h"
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <cmath>

static float distXZ(const Vec3& a,const Vec3& b) {
    float dx=a.x-b.x, dz=a.z-b.z;
    return std::sqrt(dx*dx+dz*dz);
}

int main() {
    VitaRenderer renderer;
    if(!renderer.init()) return -1;

    VitaInput controls;
    Player player;
    player.position={0,0,-4};
    Vehicle car;
    car.position={4,0,4};
    Camera camera;
    World world;
    EnvironmentSystem environment;
    SightCityMap sightMap;
    WorldCollisionSystem collisions;
    world.radius=1; // M63: conservative physical-Vita streaming radius.
    WantedSystem wanted;
    TrafficSystem traffic;
    NPCSystem npcs;
    GameSettings settings;
    sceIoMkdir("ux0:data/SightCityUnderworld", 0777);
    const char* settingsPath="ux0:data/SightCityUnderworld/settings.cfg";
    settings.load(settingsPath);

    const float dt=1.0f/30.0f; // Vita target: stable 30 fps.
    while(!controls.quitRequested()) {
        InputState in=controls.poll(settings,player.inVehicle);

        if(in.enterExitVehicle()) {
            if(player.inVehicle) {
                player.inVehicle=false;
                player.position={car.position.x+2.0f,0,car.position.z};
                if(camera.mode==CameraMode::FirstPerson && (!settings.firstPersonEnabled||!settings.firstPersonOnFoot)) camera.mode=CameraMode::ThirdNear;
            } else if(distXZ(player.position,car.position)<3.0f) {
                player.inVehicle=true;
                if(camera.mode==CameraMode::FirstPerson && (!settings.firstPersonEnabled||!settings.firstPersonInVehicle)) camera.mode=CameraMode::ThirdNear;
            }
        }

        if(in.cameraCycle()) {
            bool allow=settings.firstPersonEnabled && (player.inVehicle?settings.firstPersonInVehicle:settings.firstPersonOnFoot);
            camera.cycle(allow);
        }
        if(in.fire()) wanted.addHeat(12.0f); // Prototype hook for later combat/crime events.

        Vec3 preFocus=player.inVehicle?car.position:player.position;
        environment.stream(preFocus,3);
        sightMap.stream(preFocus,3);
        collisions.rebuild(environment,sightMap);
        player.updateWorld(in,dt,environment,collisions);
        car.update(in,dt,player.inVehicle,settings.vehicleIndicators,settings.vehicleLights);
        Vec3 focus=player.inVehicle?car.position:player.position;
        float heading=player.inVehicle?car.heading:player.heading;
        camera.follow(focus,heading,in.lookX,in.lookY,dt,player.inVehicle);
        world.updateStreaming(focus);
        RegionType region=world.regionAt((int)std::floor(focus.x/World::CellSizeMeters),(int)std::floor(focus.z/World::CellSizeMeters));
        traffic.setPoliceAlert(wanted.level>=2);
        traffic.update(focus,dt,region);
        npcs.update(focus,dt,region,wanted.level>=2);
        wanted.update(dt,false);

        renderer.draw(player,car,camera,world,wanted,traffic,npcs,30.0f);
        sceKernelDelayThread(1000); // Yield; swap buffers performs the main pacing.
    }

    settings.save(settingsPath);
    renderer.shutdown();
    sceKernelExitProcess(0);
    return 0;
}
