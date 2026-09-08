#pragma once

struct Player;
struct Vehicle;
class WantedSystem;
class EnvironmentSystem;
class WorldCollisionSystem;

struct vita2d_pgf;

class VitaDevMenu {
public:
    VitaDevMenu();
    ~VitaDevMenu();

    // Returns true while the Dev Menu owns the frame/input.
    bool update(Player& player,
                Vehicle& car,
                WantedSystem& wanted,
                const EnvironmentSystem& environment,
                const WorldCollisionSystem& collisions);

    void draw(const Player& player,
              const Vehicle& car,
              const WantedSystem& wanted) const;

    bool godMode() const { return godMode_; }
    bool flyMode() const { return flyMode_; }

private:
    enum Item {
        GodMode = 0,
        HealPlayer,
        SnapToGround,
        ClearWanted,
        TeleportToSpawn,
        FlyMode,
        CoordinatesHud,
        CloseDevMenu,
        ItemCount
    };

    bool open_;
    bool godMode_;
    bool flyMode_;
    int selected_;
    unsigned int previousButtons_;
    vita2d_pgf* font_;

    bool pressed(unsigned int buttons, unsigned int mask) const;
    void activate(Player& player,
                  Vehicle& car,
                  WantedSystem& wanted,
                  const EnvironmentSystem& environment,
                  const WorldCollisionSystem& collisions);
};
