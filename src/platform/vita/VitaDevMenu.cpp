#include "platform/vita/VitaDevMenu.h"
#include "game/Player.h"
#include "game/Vehicle.h"
#include "game/WantedSystem.h"
#include "game/EnvironmentSystem.h"
#include "game/WorldCollisionSystem.h"

#include <vita2d.h>
#include <cstdio>
#include <cstring>

namespace {
static const unsigned COL_BG    = RGBA8(8,10,16,230);
static const unsigned COL_PANEL = RGBA8(24,28,38,245);
static const unsigned COL_ROW   = RGBA8(42,46,58,235);
static const unsigned COL_SEL   = RGBA8(120,35,35,245);
static const unsigned COL_TEXT  = RGBA8(245,245,245,255);
static const unsigned COL_MUTED = RGBA8(175,180,190,255);
static const unsigned COL_ON    = RGBA8(90,220,120,255);

static const char* itemName(int i) {
    switch(i) {
        case 0: return "God Mode";
        case 1: return "Heal Player";
        case 2: return "Snap To Ground";
        case 3: return "Clear Wanted";
        case 4: return "Teleport To Spawn";
        case 5: return "Fly Mode";
        case 6: return "Close Dev Menu";
        default: return "";
    }
}
}

VitaDevMenu::VitaDevMenu()
    : open_(false), godMode_(false), flyMode_(false), selected_(0), font_(nullptr) {
    std::memset(&previous_,0,sizeof(previous_));
    font_ = vita2d_load_default_pgf();
}

VitaDevMenu::~VitaDevMenu() {
    if(font_) {
        vita2d_free_pgf((vita2d_pgf*)font_);
        font_ = nullptr;
    }
}

bool VitaDevMenu::update(Player& player,
                         Vehicle& car,
                         WantedSystem& wanted,
                         const EnvironmentSystem& environment,
                         const WorldCollisionSystem& collisions) {
    SceCtrlData pad{};
    sceCtrlPeekBufferPositive(0,&pad,1);
    const unsigned pressed = pad.buttons & ~previous_.buttons;

    if(pressed & SCE_CTRL_SELECT) open_ = !open_;

    if(godMode_) player.health = 100.0f;

    if(open_) {
        if(pressed & SCE_CTRL_UP)
            selected_ = (selected_ + ItemCount - 1) % ItemCount;
        if(pressed & SCE_CTRL_DOWN)
            selected_ = (selected_ + 1) % ItemCount;

        if(pressed & SCE_CTRL_CIRCLE)
            open_ = false;

        if(pressed & SCE_CTRL_CROSS) {
            switch(selected_) {
                case GodMode:
                    godMode_ = !godMode_;
                    if(godMode_) player.health = 100.0f;
                    break;
                case Heal:
                    player.health = 100.0f;
                    break;
                case GroundSnap: {
                    float gy = collisions.groundHeight(
                        player.position.x,player.position.z,environment);
                    player.position.y = gy;
                    break;
                }
                case ClearWanted:
                    wanted.level = 0;
                    wanted.heat = 0.0f;
                    break;
                case TeleportSpawn:
                    if(player.inVehicle) {
                        car.position = {4.0f,0.0f,4.0f};
                        player.position = car.position;
                    } else {
                        player.position = {0.0f,0.0f,-4.0f};
                    }
                    break;
                case FlyMode:
                    flyMode_ = !flyMode_;
                    break;
                case CloseMenu:
                    open_ = false;
                    break;
            }
        }
    }

    previous_ = pad;
    return open_;
}

void VitaDevMenu::draw(const Player& player,const Vehicle& car,const WantedSystem& wanted) {
    vita2d_start_drawing();
    vita2d_clear_screen();

    vita2d_draw_rectangle(0,0,960,544,COL_BG);
    vita2d_draw_rectangle(120,48,720,448,COL_PANEL);

    vita2d_pgf* pgf = (vita2d_pgf*)font_;
    if(pgf) {
        vita2d_pgf_draw_text(pgf,155,92,COL_TEXT,1.35f,
                             "SIGHT CITY: UNDERWORLD - DEV MENU");
        vita2d_pgf_draw_text(pgf,155,120,COL_MUTED,0.85f,
                             "SELECT toggle | D-Pad navigate | X select | O close");
    }

    for(int i=0;i<ItemCount;i++) {
        float y = 145.0f + i*48.0f;
        vita2d_draw_rectangle(150,y,660,38,i==selected_ ? COL_SEL : COL_ROW);

        if(pgf) {
            vita2d_pgf_draw_text(pgf,170,y+26,COL_TEXT,1.0f,itemName(i));
            if(i==GodMode) {
                vita2d_pgf_draw_text(pgf,650,y+26,
                                     godMode_ ? COL_ON : COL_MUTED,
                                     1.0f,godMode_ ? "ON" : "OFF");
            } else if(i==FlyMode) {
                vita2d_pgf_draw_text(pgf,650,y+26,
                                     flyMode_ ? COL_ON : COL_MUTED,
                                     1.0f,flyMode_ ? "ON" : "OFF");
            }
        }
    }

    if(pgf) {
        char status[256];
        const Vec3& pos = player.inVehicle ? car.position : player.position;

        // M74: explicit live world coordinates for development/debugging.
        std::snprintf(status,sizeof(status),
                      "X: %.2f   Y: %.2f   Z: %.2f",
                      pos.x,pos.y,pos.z);
        vita2d_pgf_draw_text(pgf,155,438,COL_TEXT,0.95f,status);

        std::snprintf(status,sizeof(status),
                      "Health: %.0f   Wanted: %d",
                      player.health,wanted.level);
        vita2d_pgf_draw_text(pgf,155,466,COL_MUTED,0.85f,status);
    }

    vita2d_end_drawing();
    vita2d_swap_buffers();
}
