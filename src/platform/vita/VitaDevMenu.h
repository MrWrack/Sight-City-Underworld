#pragma once

#include <psp2/ctrl.h>

class Player;
class Vehicle;
class WantedSystem;
class EnvironmentSystem;
class WorldCollisionSystem;

class VitaDevMenu {
public:
    VitaDevMenu();
    ~VitaDevMenu();

    bool update(Player& player,
                Vehicle& car,
                WantedSystem& wanted,
                const EnvironmentSystem& environment,
                const WorldCollisionSystem& collisions);

    void draw(const Player& player, const Vehicle& car, const WantedSystem& wanted);

    bool isOpen() const { return open_; }
    bool godMode() const { return godMode_; }
    bool flyMode() const { return flyMode_; }

private:
    enum Item {
        GodMode = 0,
        Heal,
        GroundSnap,
        ClearWanted,
        TeleportSpawn,
        FlyMode,
        CloseMenu,
        ItemCount
    };

    bool open_;
    bool godMode_;
    bool flyMode_;
    int selected_;
    SceCtrlData previous_;
    void* font_;
};
