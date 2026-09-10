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
#include "platform/vita/VitaDevMenu.h"
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <cmath>
#include <cstdio>
#include <vita2d.h>
#include <psp2/ctrl.h>

static float distXZ(const Vec3& a,const Vec3& b) {
    float dx=a.x-b.x, dz=a.z-b.z;
    return std::sqrt(dx*dx+dz*dz);
}

static void makeCameraRelative(InputState& in,const Camera& camera) {
    // M84: derive movement directly from the actual camera direction.
    // This avoids movement direction shifting/flipping when Dash turns
    // while the camera is orbiting around a building.
    float fx=camera.target.x-camera.position.x;
    float fz=camera.target.z-camera.position.z;
    float len=std::sqrt(fx*fx+fz*fz);

    if(len<0.0001f) {
        fx=0.0f;
        fz=1.0f;
        len=1.0f;
    }

    fx/=len;
    fz/=len;

    // Horizontal right vector from the camera forward vector.
    const float rx=fz;
    const float rz=-fx;

    const float localRight=in.moveX;
    const float localForward=in.moveY;

    in.moveX=rx*localRight + fx*localForward;
    in.moveY=rz*localRight + fz*localForward;
}


enum class M101MenuScreen { Main, Settings };

static void m101DrawButton(vita2d_pgf* font,float x,float y,float w,float h,
                           const char* text,bool selected) {
    const unsigned bg=selected?RGBA8(78,88,105,235):RGBA8(20,24,31,220);
    const unsigned border=selected?RGBA8(220,60,60,255):RGBA8(80,86,96,255);
    vita2d_draw_rectangle(x,y,w,h,bg);
    vita2d_draw_rectangle(x,y,w,2.0f,border);
    vita2d_draw_rectangle(x,y+h-2.0f,w,2.0f,border);
    if(font)
        vita2d_pgf_draw_text(font,x+20.0f,y+34.0f,RGBA8(255,255,255,255),0.92f,text);
}

static bool m101RunStartMenu(GameSettings& settings,const char* settingsPath) {
    vita2d_pgf* font=vita2d_load_default_pgf();
    M101MenuScreen screen=M101MenuScreen::Main;
    int selected=0;
    unsigned prev=0;
    bool running=true;
    bool startGame=false;

    while(running) {
        SceCtrlData pad{};
        sceCtrlPeekBufferPositive(0,&pad,1);
        const unsigned pressed=pad.buttons & ~prev;

        if(screen==M101MenuScreen::Main) {
            if(pressed&SCE_CTRL_UP)   { selected=(selected+2)%3; }
            if(pressed&SCE_CTRL_DOWN) { selected=(selected+1)%3; }

            if(pressed&SCE_CTRL_CROSS) {
                if(selected==0) {
                    startGame=true;
                    running=false;
                } else if(selected==1) {
                    screen=M101MenuScreen::Settings;
                    selected=0;
                } else {
                    running=false;
                    startGame=false;
                }
            }
        } else {
            if(pressed&SCE_CTRL_UP)   { selected=(selected+3)%4; }
            if(pressed&SCE_CTRL_DOWN) { selected=(selected+1)%4; }

            if(pressed&SCE_CTRL_LEFT || pressed&SCE_CTRL_RIGHT || pressed&SCE_CTRL_CROSS) {
                if(selected==0) {
                    settings.lookSensitivity += (pressed&SCE_CTRL_LEFT)?-0.1f:0.1f;
                    if(settings.lookSensitivity<0.5f) settings.lookSensitivity=0.5f;
                    if(settings.lookSensitivity>2.0f) settings.lookSensitivity=2.0f;
                } else if(selected==1) {
                    settings.invertCameraY=!settings.invertCameraY;
                } else if(selected==2) {
                    settings.firstPersonEnabled=!settings.firstPersonEnabled;
                } else if(selected==3 && (pressed&SCE_CTRL_CROSS)) {
                    settings.save(settingsPath);
                    screen=M101MenuScreen::Main;
                    selected=1;
                }
            }
            if(pressed&SCE_CTRL_CIRCLE) {
                settings.save(settingsPath);
                screen=M101MenuScreen::Main;
                selected=1;
            }
        }

        prev=pad.buttons;

        vita2d_start_drawing();
        vita2d_clear_screen();

        // Dark city-style start menu.
        vita2d_draw_rectangle(0,0,960,544,RGBA8(8,12,18,255));
        vita2d_draw_rectangle(0,0,960,7,RGBA8(195,43,43,255));
        if(font) {
            vita2d_pgf_draw_text(font,70,92,RGBA8(255,255,255,255),1.55f,"SIGHT CITY");
            vita2d_pgf_draw_text(font,70,132,RGBA8(210,50,50,255),1.05f,"UNDERWORLD");
        }

        if(screen==M101MenuScreen::Main) {
            m101DrawButton(font,70,205,330,52,"START GAME",selected==0);
            m101DrawButton(font,70,270,330,52,"SETTINGS",selected==1);
            m101DrawButton(font,70,335,330,52,"EXIT",selected==2);
            if(font)
                vita2d_pgf_draw_text(font,70,430,RGBA8(180,190,202,255),0.72f,
                                     "D-Pad: Select   X: Confirm");
        } else {
            char line[96];
            std::snprintf(line,sizeof(line),"LOOK SENSITIVITY  %.1f",settings.lookSensitivity);
            m101DrawButton(font,70,190,430,48,line,selected==0);
            std::snprintf(line,sizeof(line),"INVERT CAMERA Y  %s",settings.invertCameraY?"ON":"OFF");
            m101DrawButton(font,70,247,430,48,line,selected==1);
            std::snprintf(line,sizeof(line),"FIRST PERSON  %s",settings.firstPersonEnabled?"ON":"OFF");
            m101DrawButton(font,70,304,430,48,line,selected==2);
            m101DrawButton(font,70,361,430,48,"BACK",selected==3);
            if(font)
                vita2d_pgf_draw_text(font,70,450,RGBA8(180,190,202,255),0.70f,
                                     "Left/Right or X: Change   O: Back");
        }

        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceKernelDelayThread(16000);
    }

    if(font) vita2d_free_pgf(font);
    return startGame;
}

int main() {
    VitaRenderer renderer;
    if(!renderer.init()) return -1;

    VitaInput controls;
    VitaDevMenu devMenu; // Temporary dev menu.
    Player player;
    player.position={0.28f,2.20f,7.92f}; // M80 spawn position
    Vehicle car;
    car.position={4,0,4};
    Camera camera;
    World world;
    EnvironmentSystem environment;
    SightCityMap sightMap;
    WorldCollisionSystem collisions;
    world.radius=2;
    WantedSystem wanted;
    TrafficSystem traffic;
    NPCSystem npcs;
    GameSettings settings;
    sceIoMkdir("ux0:data/SightCityUnderworld", 0777);
    const char* settingsPath="ux0:data/SightCityUnderworld/settings.cfg";
    settings.load(settingsPath);

    // M101 real start menu. START GAME no longer opens Settings.
    // SETTINGS is a separate menu item.
    if(!m101RunStartMenu(settings,settingsPath)) {
        renderer.shutdown();
        sceKernelExitProcess(0);
        return 0;
    }

    // M101: build collision data first, then put Dash exactly on the real floor.
    environment.stream(player.position,3);
    sightMap.stream(player.position,3);
    collisions.rebuild(environment,sightMap);
    player.position.y=collisions.groundHeight(player.position.x,player.position.z,environment);
    player.velocity={0.0f,0.0f,0.0f};

    const float dt=1.0f/30.0f; // Vita target: stable 30 fps.
    while(!controls.quitRequested()) {
        // Temporary developer menu. SELECT opens/closes it.
        if(devMenu.update(player,car,wanted,environment,collisions)) {
            devMenu.draw(player,car,wanted);
            sceKernelDelayThread(1000);
            continue;
        }

        InputState in=controls.poll(settings,player.inVehicle);

        // M84 temporary fly controls:
        // X = rise, Circle = descend.
        // Horizontal fly movement follows the camera just like normal movement.
        if(devMenu.flyMode() && !player.inVehicle) {
            const float flyHorizontalSpeed = 8.0f;
            const float flyVerticalSpeed = 6.0f;

            InputState flyMove=in;
            makeCameraRelative(flyMove,camera);
            player.position.x += flyMove.moveX * flyHorizontalSpeed * dt;
            player.position.z += flyMove.moveY * flyHorizontalSpeed * dt;

            if(in.sprint())  player.position.y += flyVerticalSpeed * dt;   // X = up
            if(in.stealth()) player.position.y -= flyVerticalSpeed * dt;   // Circle = down

            player.velocity = {0,0,0};
        }

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
        if(!devMenu.flyMode()) {
            if(!player.inVehicle) {
                // M84: movement uses the actual camera forward/right vectors.
                // Keep camera controls untouched; this change only affects movement.
                const float cameraWorldAngle=player.heading+camera.yaw;
                makeCameraRelative(in,camera);
                player.updateWorld(in,dt,environment,collisions);
                camera.yaw=cameraWorldAngle-player.heading;
            } else {
                player.updateWorld(in,dt,environment,collisions);
            }
        }
        // M102: after normal on-foot physics, keep Dash exactly on collision ground.
        // Fly Mode is intentionally excluded.
        if(!devMenu.flyMode() && !player.inVehicle) {
            const float groundY=collisions.groundHeight(player.position.x,player.position.z,environment);
            if(std::fabs(player.position.y-groundY)<1.25f || player.position.y<groundY) {
                player.position.y=groundY;
            }
        }

        if(devMenu.godMode()) player.health=100.0f;
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
