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

void groundTile(float x0, float z0, float x1, float z1,
                const Camera& cam, unsigned color) {
    quadPool(
        {x0,0.0f,z0},
        {x1,0.0f,z0},
        {x1,0.0f,z1},
        {x0,0.0f,z1},
        cam,color
    );
}

void drawExpandedGround(const Camera& cam) {
    // M83: larger neighborhood around the current physical-Vita test area.
    // Tiled instead of one huge quad so near-plane rejection does not remove
    // the whole ground when one corner is behind the camera.
    const unsigned grassA = RGBA8(72, 94, 68, 255);
    const unsigned grassB = RGBA8(78, 101, 72, 255);

    constexpr float TILE = 20.0f;
    for (int z=-4; z<6; ++z) {
        for (int x=-4; x<5; ++x) {
            float x0 = x*TILE;
            float z0 = z*TILE;
            unsigned col = ((x+z)&1) ? grassA : grassB;
            groundTile(x0,z0,x0+TILE,z0+TILE,cam,col);
        }
    }
}

void roadX(float z, const Camera& cam) {
    const unsigned asphalt = RGBA8(42,44,47,255);
    const unsigned line = RGBA8(218,205,116,255);

    // Road from x -80 to +100.
    for (int i=0;i<9;i++) {
        float x0=-80.0f+i*20.0f;
        float x1=x0+20.0f;
        groundTile(x0,z-4.0f,x1,z+4.0f,cam,asphalt);
    }

    for (float x=-76.0f;x<96.0f;x+=10.0f) {
        groundTile(x,z-0.08f,x+5.0f,z+0.08f,cam,line);
    }
}

void roadZ(float x, const Camera& cam) {
    const unsigned asphalt = RGBA8(42,44,47,255);
    const unsigned line = RGBA8(218,205,116,255);

    // Road from z -80 to +120.
    for (int i=0;i<10;i++) {
        float z0=-80.0f+i*20.0f;
        float z1=z0+20.0f;
        groundTile(x-4.0f,z0,x+4.0f,z1,cam,asphalt);
    }

    for (float z=-76.0f;z<116.0f;z+=10.0f) {
        groundTile(x-0.08f,z,x+0.08f,z+5.0f,cam,line);
    }
}

void simpleWindowsFront(const Vec3& c,float sx,float sy,float sz,
                        const Camera& cam,unsigned glass) {
    if (sy < 5.0f) return;

    const float frontZ = c.z - sz*0.5f - 0.02f;
    int floors = (int)(sy / 3.0f);
    if (floors > 5) floors = 5;

    for (int f=0; f<floors; ++f) {
        float y = 1.0f + f*2.6f;
        if (y+0.9f > sy-0.35f) break;

        quadPool(
            {c.x-sx*0.34f,y,frontZ},
            {c.x-sx*0.10f,y,frontZ},
            {c.x-sx*0.10f,y+0.9f,frontZ},
            {c.x-sx*0.34f,y+0.9f,frontZ},
            cam,glass
        );

        quadPool(
            {c.x+sx*0.10f,y,frontZ},
            {c.x+sx*0.34f,y,frontZ},
            {c.x+sx*0.34f,y+0.9f,frontZ},
            {c.x+sx*0.10f,y+0.9f,frontZ},
            cam,glass
        );
    }
}

void cityBuilding(const Vec3& c,float sx,float sy,float sz,
                  const Camera& cam,unsigned front,unsigned side,unsigned top) {
    boxPool(c,sx,sy,sz,cam,front,side,top);

    // Simple GPU-cheap windows. Still pure geometry: no images or textures.
    simpleWindowsFront(
        c,sx,sy,sz,cam,
        RGBA8(62,91,112,255)
    );

    // Small rooftop block on taller buildings.
    if (sy >= 11.0f) {
        boxPool(
            {c.x+sx*0.12f,c.y+sy,c.z-sz*0.08f},
            sx*0.28f,0.75f,sz*0.24f,
            cam,
            RGBA8(112,113,116,255),
            RGBA8(82,84,87,255),
            RGBA8(143,144,146,255)
        );
    }
}

void drawExpandedBuildings(const Camera& cam) {
    // M83 neighborhood: more buildings without returning to the old unsafe
    // stack-backed vita2d_draw_array path.
    struct B {
        float x,z,sx,sy,sz;
        unsigned front,side,top;
    };

    static const B buildings[] = {
        // Around spawn / original test building.
        {  0.0f, 17.0f,  8.0f,  8.0f,  8.0f, RGBA8(150,142,145,255), RGBA8(110,104,108,255), RGBA8(180,174,177,255)},
        {-14.0f, 18.0f, 10.0f, 11.0f,  9.0f, RGBA8(139,145,151,255), RGBA8( 98,106,114,255), RGBA8(170,176,181,255)},
        { 15.0f, 18.0f, 11.0f,  7.0f, 10.0f, RGBA8(164,143,123,255), RGBA8(119, 99, 83,255), RGBA8(190,169,147,255)},

        // Next block.
        {-31.0f, 20.0f, 12.0f, 14.0f, 11.0f, RGBA8(126,136,148,255), RGBA8( 88, 98,109,255), RGBA8(156,165,176,255)},
        { 32.0f, 20.0f, 12.0f, 12.0f, 11.0f, RGBA8(153,150,139,255), RGBA8(108,106, 96,255), RGBA8(180,177,165,255)},
        {-47.0f, 18.0f, 10.0f,  8.0f, 12.0f, RGBA8(145,129,121,255), RGBA8(104, 90, 84,255), RGBA8(174,156,147,255)},
        { 48.0f, 18.0f, 11.0f, 16.0f, 10.0f, RGBA8(122,132,137,255), RGBA8( 86, 95,100,255), RGBA8(153,163,168,255)},

        // Deeper into the expanded area.
        {-14.0f, 39.0f, 11.0f, 10.0f, 10.0f, RGBA8(156,145,132,255), RGBA8(111,100, 89,255), RGBA8(184,173,158,255)},
        { 14.0f, 39.0f, 10.0f, 18.0f, 11.0f, RGBA8(126,137,149,255), RGBA8( 88, 98,110,255), RGBA8(157,167,178,255)},
        {-32.0f, 40.0f, 12.0f,  9.0f, 12.0f, RGBA8(151,138,134,255), RGBA8(106, 94, 91,255), RGBA8(180,166,161,255)},
        { 33.0f, 40.0f, 11.0f, 13.0f, 12.0f, RGBA8(137,142,143,255), RGBA8( 96,101,102,255), RGBA8(166,171,171,255)},

        {-48.0f, 42.0f, 12.0f, 17.0f, 12.0f, RGBA8(130,139,145,255), RGBA8( 91, 99,106,255), RGBA8(160,169,175,255)},
        { 49.0f, 42.0f, 12.0f, 10.0f, 12.0f, RGBA8(162,145,127,255), RGBA8(116,100, 85,255), RGBA8(191,172,151,255)},

        // Far block / skyline.
        {-28.0f, 65.0f, 14.0f, 22.0f, 14.0f, RGBA8(119,130,142,255), RGBA8( 83, 93,104,255), RGBA8(150,160,171,255)},
        { -8.0f, 67.0f, 13.0f, 28.0f, 13.0f, RGBA8(138,139,144,255), RGBA8( 96, 98,104,255), RGBA8(169,170,175,255)},
        { 11.0f, 66.0f, 14.0f, 24.0f, 14.0f, RGBA8(128,137,146,255), RGBA8( 89, 98,107,255), RGBA8(159,168,177,255)},
        { 31.0f, 64.0f, 13.0f, 19.0f, 14.0f, RGBA8(154,145,135,255), RGBA8(109,101, 92,255), RGBA8(183,174,162,255)}
    };

    for (const B& b : buildings) {
        cityBuilding(
            {b.x,0.0f,b.z},
            b.sx,b.sy,b.sz,
            cam,b.front,b.side,b.top
        );
    }
}

void drawTestCity(const Camera& cam) {
    drawExpandedGround(cam);

    // Two crossing streets make the larger space easier to navigate and
    // visually show that the playable test area was expanded.
    roadX(7.0f,cam);
    roadX(52.0f,cam);
    roadZ(-22.0f,cam);
    roadZ(22.0f,cam);

    drawExpandedBuildings(cam);
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

    // M83 original Vita expansion: pure geometry only.
    // Larger ground, roads and multiple buildings.
    drawTestCity(camera);

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
