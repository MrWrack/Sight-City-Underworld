#include "platform/vita/VitaRenderer.h"
#include <vita2d.h>

bool VitaRenderer::init() {
    if (vita2d_init() < 0) {
        return false;
    }

    // M64 diagnostic clear color.
    vita2d_set_clear_color(RGBA8(18, 24, 34, 255));
    return true;
}

void VitaRenderer::shutdown() {
    vita2d_fini();
}

void VitaRenderer::draw(const Player& player,
                        const Vehicle& car,
                        const Camera& camera,
                        const World& world,
                        const WantedSystem& wanted,
                        const TrafficSystem& traffic,
                        const NPCSystem& npcs,
                        float fps) {
    // M64 GPU isolation test:
    // Deliberately do NOT render world geometry, traffic, NPCs,
    // vehicles, custom triangles, or projected 3D geometry.
    vita2d_start_drawing();
    vita2d_clear_screen();

    // One simple Vita2D primitive. If this runs on hardware,
    // Vita2D init/frame/swap are healthy and the crash is in
    // the removed world/custom geometry path.
    vita2d_draw_rectangle(
        330.0f,
        220.0f,
        300.0f,
        104.0f,
        RGBA8(40, 145, 220, 255)
    );

    // Small center marker using another known-safe primitive.
    vita2d_draw_rectangle(
        474.0f,
        266.0f,
        12.0f,
        12.0f,
        RGBA8(255, 255, 255, 255)
    );

    vita2d_end_drawing();
    vita2d_swap_buffers();

    // Keep the normal public draw signature without touching game state.
    (void)player;
    (void)car;
    (void)camera;
    (void)world;
    (void)wanted;
    (void)traffic;
    (void)npcs;
    (void)fps;
}
