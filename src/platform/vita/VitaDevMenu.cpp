#include "platform/vita/VitaDevMenu.h"
#include "platform/vita/DevDebugState.h"

#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/WantedSystem.h"
#include "game/EnvironmentSystem.h"
#include "game/WorldCollisionSystem.h"

#include <psp2/ctrl.h>
#include <vita2d.h>
#include <cstdio>

VitaDevMenu::VitaDevMenu()
    : open_(false),
      godMode_(false),
      flyMode_(false),
      selected_(0),
      previousButtons_(0),
      font_(nullptr) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    font_ = vita2d_load_default_pgf();
}

VitaDevMenu::~VitaDevMenu() {
    if(font_) {
        vita2d_free_pgf(font_);
        font_ = nullptr;
    }
}

bool VitaDevMenu::pressed(unsigned int buttons, unsigned int mask) const {
    return (buttons & mask) && !(previousButtons_ & mask);
}

void VitaDevMenu::activate(Player& player,
                           Vehicle& car,
                           WantedSystem& wanted,
                           const EnvironmentSystem& environment,
                           const WorldCollisionSystem& collisions) {
    switch(selected_) {
        case GodMode:
            godMode_ = !godMode_;
            if(godMode_) player.health = 100.0f;
            break;

        case HealPlayer:
            player.health = 100.0f;
            break;

        case SnapToGround:
            player.position.y =
                collisions.groundHeight(player.position.x, player.position.z, environment);
            player.velocity = {0.0f, 0.0f, 0.0f};
            break;

        case ClearWanted:
            wanted.level = 0;
            wanted.heat = 0.0f;
            break;

        case TeleportToSpawn:
            player.inVehicle = false;
            player.position = {0.28f, 2.20f, 7.92f};
            player.velocity = {0.0f, 0.0f, 0.0f};
            car.position = {4.0f, 0.0f, 4.0f};
            break;

        case FlyMode:
            flyMode_ = !flyMode_;
            player.velocity = {0.0f, 0.0f, 0.0f};
            break;

        case CoordinatesHud:
            DevDebugState::toggleCoordinatesHud();
            break;

        case CloseDevMenu:
            open_ = false;
            break;

        default:
            break;
    }
}

bool VitaDevMenu::update(Player& player,
                         Vehicle& car,
                         WantedSystem& wanted,
                         const EnvironmentSystem& environment,
                         const WorldCollisionSystem& collisions) {
    SceCtrlData pad{};
    sceCtrlPeekBufferPositive(0, &pad, 1);
    const unsigned int buttons = pad.buttons;

    // SELECT always toggles Dev Menu.
    if(pressed(buttons, SCE_CTRL_SELECT)) {
        open_ = !open_;
    }

    if(open_) {
        if(pressed(buttons, SCE_CTRL_UP)) {
            selected_--;
            if(selected_ < 0) selected_ = ItemCount - 1;
        }

        if(pressed(buttons, SCE_CTRL_DOWN)) {
            selected_++;
            if(selected_ >= ItemCount) selected_ = 0;
        }

        if(pressed(buttons, SCE_CTRL_CROSS)) {
            activate(player, car, wanted, environment, collisions);
        }

        // Circle closes the menu, except Fly Mode vertical control is only used
        // during gameplay when the menu itself is closed.
        if(pressed(buttons, SCE_CTRL_CIRCLE)) {
            open_ = false;
        }
    }

    previousButtons_ = buttons;

    if(godMode_) {
        player.health = 100.0f;
    }

    return open_;
}

void VitaDevMenu::draw(const Player& player,
                       const Vehicle& car,
                       const WantedSystem& wanted) const {
    if(!open_) return;

    vita2d_start_drawing();
    vita2d_clear_screen();

    const float x = 24.0f;
    const float y = 26.0f;
    const float w = 430.0f;
    const float h = 472.0f;

    vita2d_draw_rectangle(x, y, w, h, RGBA8(8, 12, 17, 242));
    vita2d_draw_rectangle(x, y, w, 3.0f, RGBA8(210, 50, 50, 255));

    if(font_) {
        vita2d_pgf_draw_text(font_, x + 18.0f, y + 31.0f,
                             RGBA8(255,255,255,255), 1.0f, "DEV MODE");

        const float firstY = y + 70.0f;
        const float rowH = 39.0f;

        for(int i = 0; i < ItemCount; ++i) {
            const float rowY = firstY + i * rowH;

            if(i == selected_) {
                vita2d_draw_rectangle(x + 12.0f, rowY - 24.0f,
                                      w - 24.0f, 31.0f,
                                      RGBA8(58, 70, 86, 245));
            }

            char label[96];
            switch(i) {
                case GodMode:
                    std::snprintf(label, sizeof(label),
                                  "God Mode: %s", godMode_ ? "ON" : "OFF");
                    break;
                case HealPlayer:
                    std::snprintf(label, sizeof(label), "Heal Player");
                    break;
                case SnapToGround:
                    std::snprintf(label, sizeof(label), "Snap To Ground");
                    break;
                case ClearWanted:
                    std::snprintf(label, sizeof(label), "Clear Wanted");
                    break;
                case TeleportToSpawn:
                    std::snprintf(label, sizeof(label), "Teleport To Spawn");
                    break;
                case FlyMode:
                    std::snprintf(label, sizeof(label),
                                  "Fly Mode: %s", flyMode_ ? "ON" : "OFF");
                    break;
                case CoordinatesHud:
                    std::snprintf(label, sizeof(label),
                                  "Coordinates HUD: %s",
                                  DevDebugState::coordinatesHudEnabled() ? "ON" : "OFF");
                    break;
                case CloseDevMenu:
                    std::snprintf(label, sizeof(label), "Close Dev Menu");
                    break;
                default:
                    label[0] = '\0';
                    break;
            }

            vita2d_pgf_draw_text(font_, x + 24.0f, rowY,
                                 RGBA8(244,244,244,255), 0.82f, label);
        }

        char info[96];
        std::snprintf(info, sizeof(info),
                      "Health %.0f   Wanted %d",
                      player.health, wanted.level);
        vita2d_pgf_draw_text(font_, x + 18.0f, y + h - 38.0f,
                             RGBA8(205,220,232,255), 0.72f, info);

        vita2d_pgf_draw_text(font_, x + 18.0f, y + h - 16.0f,
                             RGBA8(150,170,185,255), 0.60f,
                             "SELECT Close  |  X Select  |  O Back");
    }

    vita2d_end_drawing();
    vita2d_swap_buffers();

    (void)car;
}
