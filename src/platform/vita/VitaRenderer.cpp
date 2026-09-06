#include "platform/vita/VitaRenderer.h"
#include <vita2d.h>
#include <cmath>

namespace {

constexpr float W = 960.0f;
constexpr float H = 544.0f;
constexpr float FOCAL = 620.0f;

struct P2 {
    float x, y, z;
    bool ok;
};

Vec3 sub3(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

float dot3(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross3(const Vec3& a, const Vec3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

P2 projectSafe(const Vec3& p, const Camera& cam) {
    Vec3 f = normalize(sub3(cam.target, cam.position));
    Vec3 up{0.0f, 1.0f, 0.0f};
    Vec3 r = normalize(cross3(f, up));
    Vec3 u = cross3(r, f);
    Vec3 rel = sub3(p, cam.position);

    float cx = dot3(rel, r);
    float cy = dot3(rel, u);
    float cz = dot3(rel, f);

    if (!std::isfinite(cx) || !std::isfinite(cy) ||
        !std::isfinite(cz) || cz < 0.25f) {
        return {0, 0, cz, false};
    }

    float sx = W * 0.5f + cx * FOCAL / cz;
    float sy = H * 0.52f - cy * FOCAL / cz;

    if (!std::isfinite(sx) || !std::isfinite(sy) ||
        sx < -2048.0f || sx > 2048.0f ||
        sy < -2048.0f || sy > 2048.0f) {
        return {0, 0, cz, false};
    }

    return {sx, sy, cz, true};
}

void triSafe(const P2& a, const P2& b, const P2& c, unsigned color) {
    if (!a.ok || !b.ok || !c.ok) return;

    vita2d_color_vertex v[3] = {
        {a.x, a.y, 0.5f, color},
        {b.x, b.y, 0.5f, color},
        {c.x, c.y, 0.5f, color}
    };

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, v, 3);
}

void quadSafe(const Vec3& a, const Vec3& b,
              const Vec3& c, const Vec3& d,
              const Camera& cam, unsigned color) {
    P2 pa = projectSafe(a, cam);
    P2 pb = projectSafe(b, cam);
    P2 pc = projectSafe(c, cam);
    P2 pd = projectSafe(d, cam);

    triSafe(pa, pb, pc, color);
    triSafe(pa, pc, pd, color);
}

void oneBuilding(const Camera& cam) {
    // Fixed diagnostic building positioned in front of the starting area.
    const float x0 = -3.0f;
    const float x1 =  3.0f;
    const float y0 =  0.0f;
    const float y1 =  6.0f;
    const float z0 =  8.0f;
    const float z1 = 14.0f;

    const unsigned front = RGBA8(145, 150, 158, 255);
    const unsigned side  = RGBA8(105, 112, 122, 255);
    const unsigned roof  = RGBA8(180, 184, 188, 255);

    quadSafe({x0,y0,z0},{x1,y0,z0},{x1,y1,z0},{x0,y1,z0},cam,front);
    quadSafe({x1,y0,z0},{x1,y0,z1},{x1,y1,z1},{x1,y1,z0},cam,side);
    quadSafe({x1,y0,z1},{x0,y0,z1},{x0,y1,z1},{x1,y1,z1},cam,front);
    quadSafe({x0,y0,z1},{x0,y0,z0},{x0,y1,z0},{x0,y1,z1},cam,side);
    quadSafe({x0,y1,z0},{x1,y1,z0},{x1,y1,z1},{x0,y1,z1},cam,roof);
}

void diagnosticGround(const Camera& cam) {
    // One 24x24 m ground quad only. No streaming world yet.
    quadSafe(
        {-12.0f, 0.0f, -2.0f},
        { 12.0f, 0.0f, -2.0f},
        { 12.0f, 0.0f, 22.0f},
        {-12.0f, 0.0f, 22.0f},
        cam,
        RGBA8(72, 96, 68, 255)
    );
}

} // namespace

bool VitaRenderer::init() {
    if (vita2d_init() < 0) return false;
    vita2d_set_clear_color(RGBA8(110, 170, 215, 255));
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
    vita2d_start_drawing();
    vita2d_clear_screen();

    // M65: exactly one ground quad + one building.
    diagnosticGround(camera);
    oneBuilding(camera);

    // Tiny 2D marker proves the frame reached the HUD stage.
    vita2d_draw_rectangle(476.0f, 270.0f, 8.0f, 8.0f,
                          RGBA8(255,255,255,255));

    vita2d_end_drawing();
    vita2d_swap_buffers();

    (void)player;
    (void)car;
    (void)world;
    (void)wanted;
    (void)traffic;
    (void)npcs;
    (void)fps;
}
