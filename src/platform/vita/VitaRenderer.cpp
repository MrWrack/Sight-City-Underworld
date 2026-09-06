#include "platform/vita/VitaRenderer.h"
#include <vita2d.h>
#include <cmath>

namespace {

constexpr float W = 960.0f;
constexpr float H = 544.0f;
constexpr float FOCAL = 560.0f;

struct P2 {
    float x, y, z;
    bool ok;
};

Vec3 sub3(const Vec3& a, const Vec3& b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}

float dot3(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross3(const Vec3& a, const Vec3& b) {
    return {
        a.y*b.z-a.z*b.y,
        a.z*b.x-a.x*b.z,
        a.x*b.y-a.y*b.x
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
        !std::isfinite(cz) || cz < 0.30f) {
        return {0,0,cz,false};
    }

    float sx = W*0.5f + cx * FOCAL / cz;
    float sy = H*0.54f - cy * FOCAL / cz;

    if (!std::isfinite(sx) || !std::isfinite(sy) ||
        sx < -2048.0f || sx > 2048.0f ||
        sy < -2048.0f || sy > 2048.0f) {
        return {0,0,cz,false};
    }

    return {sx, sy, cz, true};
}

void triPool(const P2& a, const P2& b, const P2& c, unsigned color) {
    if (!a.ok || !b.ok || !c.ok) return;

    vita2d_color_vertex* v =
        (vita2d_color_vertex*)vita2d_pool_memalign(
            3 * sizeof(vita2d_color_vertex),
            sizeof(vita2d_color_vertex));

    if (!v) return;

    v[0] = {a.x, a.y, 0.5f, color};
    v[1] = {b.x, b.y, 0.5f, color};
    v[2] = {c.x, c.y, 0.5f, color};

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, v, 3);
}

void quadPool(const Vec3& a, const Vec3& b,
              const Vec3& c, const Vec3& d,
              const Camera& cam, unsigned color) {
    P2 pa = projectSafe(a, cam);
    P2 pb = projectSafe(b, cam);
    P2 pc = projectSafe(c, cam);
    P2 pd = projectSafe(d, cam);

    triPool(pa, pb, pc, color);
    triPool(pa, pc, pd, color);
}

void boxPool(const Vec3& c,
             float sx, float sy, float sz,
             const Camera& cam,
             unsigned front,
             unsigned side,
             unsigned top) {
    float x0 = c.x - sx*0.5f;
    float x1 = c.x + sx*0.5f;
    float y0 = c.y;
    float y1 = c.y + sy;
    float z0 = c.z - sz*0.5f;
    float z1 = c.z + sz*0.5f;

    quadPool({x0,y0,z0},{x1,y0,z0},{x1,y1,z0},{x0,y1,z0},cam,front);
    quadPool({x1,y0,z0},{x1,y0,z1},{x1,y1,z1},{x1,y1,z0},cam,side);
    quadPool({x1,y0,z1},{x0,y0,z1},{x0,y1,z1},{x1,y1,z1},cam,front);
    quadPool({x0,y0,z1},{x0,y0,z0},{x0,y1,z0},{x0,y1,z1},cam,side);
    quadPool({x0,y1,z0},{x1,y1,z0},{x1,y1,z1},{x0,y1,z1},cam,top);
}

void drawGround(const Camera& cam) {
    const unsigned ground = RGBA8(78, 92, 72, 255);

    // Small, low-cost ground plane for original Vita hardware.
    quadPool(
        {-16.0f, 0.0f, -4.0f},
        { 16.0f, 0.0f, -4.0f},
        { 16.0f, 0.0f, 28.0f},
        {-16.0f, 0.0f, 28.0f},
        cam,
        ground
    );
}

void drawSimpleBuilding(const Camera& cam) {
    // One clearly visible block building.
    boxPool(
        {0.0f, 0.0f, 12.0f},
        8.0f, 7.0f, 8.0f,
        cam,
        RGBA8(145,145,150,255),
        RGBA8(105,108,115,255),
        RGBA8(175,175,180,255)
    );

    // Simple door.
    boxPool(
        {0.0f, 0.0f, 7.92f},
        1.7f, 2.5f, 0.14f,
        cam,
        RGBA8(70,55,45,255),
        RGBA8(55,45,38,255),
        RGBA8(90,75,62,255)
    );

    // Simple windows, no textures/images.
    const unsigned glass = RGBA8(55,90,120,255);
    const float z = 7.90f;

    quadPool({-2.8f,1.2f,z},{-1.2f,1.2f,z},{-1.2f,2.5f,z},{-2.8f,2.5f,z},cam,glass);
    quadPool({ 1.2f,1.2f,z},{ 2.8f,1.2f,z},{ 2.8f,2.5f,z},{ 1.2f,2.5f,z},cam,glass);
    quadPool({-2.8f,4.0f,z},{-1.2f,4.0f,z},{-1.2f,5.3f,z},{-2.8f,5.3f,z},cam,glass);
    quadPool({ 1.2f,4.0f,z},{ 2.8f,4.0f,z},{ 2.8f,5.3f,z},{ 1.2f,5.3f,z},cam,glass);
}

} // namespace

bool VitaRenderer::init() {
    if (vita2d_init() < 0) return false;

    // Solid clear color only. No textures/images.
    vita2d_set_clear_color(RGBA8(110, 175, 225, 255));
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

    // M66 original Vita test: pure geometry only.
    drawGround(camera);
    drawSimpleBuilding(camera);

    // Tiny center marker.
    vita2d_draw_rectangle(
        477.0f, 269.0f, 6.0f, 6.0f,
        RGBA8(255,255,255,255)
    );

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
