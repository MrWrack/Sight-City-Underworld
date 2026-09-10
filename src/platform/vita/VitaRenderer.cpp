#include "platform/vita/VitaRenderer.h"
#include <vita2d.h>
#include <cmath>
#include "platform/vita/DevDebugState.h"
#include <cstdio>

#include "platform/vita/M102WorldAtlas.h"
namespace {

static vita2d_texture* gM100WorldAtlas = nullptr;
static float gM102WorldBaseY=0.0f;
static bool gM102WorldBaseCaptured=false;

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

enum M99Tile {
    M99_GRASS=0, M99_ASPHALT=1, M99_CONCRETE=2, M99_DIRT=3,
    M99_BRICK=4, M99_WALL=5, M99_WINDOWS=6, M99_BARK=7,
    M99_LEAVES=8, M99_SAND=9, M99_ROCK=10, M99_METAL=11,
    M99_SIDEWALK=12, M99_ROOF=13, M99_DRY_GRASS=14, M99_WHITE=15
};

static void m99TileUV(int tile,float& u0,float& v0,float& u1,float& v1) {
    const int tx=tile&3;
    const int ty=(tile>>2)&3;
    const float step=0.25f;
    // Tiny inset avoids sampling a neighbor tile with linear filtering.
    const float inset=0.0012f;
    u0=tx*step+inset; v0=ty*step+inset;
    u1=(tx+1)*step-inset; v1=(ty+1)*step-inset;
}

static void m99TexturedQuad(const Vec3& a,const Vec3& b,
                            const Vec3& c,const Vec3& d,
                            const Camera& cam,int tile,
                            unsigned fallbackColor) {
    if(!gM100WorldAtlas) {
        quadPool(a,b,c,d,cam,fallbackColor);
        return;
    }

    const P2 pa=projectSafe(a,cam);
    const P2 pb=projectSafe(b,cam);
    const P2 pc=projectSafe(c,cam);
    const P2 pd=projectSafe(d,cam);
    if(!pa.ok || !pb.ok || !pc.ok || !pd.ok) return;

    float u0,v0,u1,v1;
    m99TileUV(tile,u0,v0,u1,v1);

    // GPU-readable Vita2D pool memory. Never stack-backed vertices.
    vita2d_texture_vertex* v =
        (vita2d_texture_vertex*)vita2d_pool_memalign(
            6*sizeof(vita2d_texture_vertex),
            sizeof(vita2d_texture_vertex));
    if(!v) return;

    // Triangle 1
    v[0].x=pa.x; v[0].y=pa.y; v[0].z=0.5f; v[0].u=u0; v[0].v=v0;
    v[1].x=pb.x; v[1].y=pb.y; v[1].z=0.5f; v[1].u=u1; v[1].v=v0;
    v[2].x=pc.x; v[2].y=pc.y; v[2].z=0.5f; v[2].u=u1; v[2].v=v1;
    // Triangle 2
    v[3].x=pa.x; v[3].y=pa.y; v[3].z=0.5f; v[3].u=u0; v[3].v=v0;
    v[4].x=pc.x; v[4].y=pc.y; v[4].z=0.5f; v[4].u=u1; v[4].v=v1;
    v[5].x=pd.x; v[5].y=pd.y; v[5].z=0.5f; v[5].u=u0; v[5].v=v1;

    vita2d_draw_array_textured(
        gM100WorldAtlas,SCE_GXM_PRIMITIVE_TRIANGLES,v,6,
        static_cast<unsigned>(RGBA8(255,255,255,255)));
}

static void m99GroundTile(float x0,float z0,float x1,float z1,
                          const Camera& cam,int tile,unsigned fallbackColor) {
    m99TexturedQuad({x0,gM102WorldBaseY+0.002f,z0},{x1,gM102WorldBaseY+0.002f,z0},
                    {x1,gM102WorldBaseY+0.002f,z1},{x0,gM102WorldBaseY+0.002f,z1},
                    cam,tile,fallbackColor);
}

static int m99GroundTileForRegion(int region) {
    if(region==1) return M99_SAND;
    if(region==2) return M99_DRY_GRASS;
    if(region==6) return M99_CONCRETE;
    return M99_GRASS;
}

static void m99BuildingTextureOverlay(const Vec3& c,float sx,float sy,float sz,
                                      const Camera& cam,unsigned seed) {
    const float x0=c.x-sx*.5f, x1=c.x+sx*.5f;
    const float y0=c.y+0.05f, y1=c.y+sy-0.08f;
    const float z0=c.z-sz*.5f, z1=c.z+sz*.5f;
    const float e=0.016f;
    const int tile=(seed&1u)?M99_BRICK:M99_WALL;
    const unsigned fallback=static_cast<unsigned>(RGBA8(155,145,138,255));

    // All four walls now receive texture instead of only front/back.
    m99TexturedQuad({x0,y0,z0-e},{x1,y0,z0-e},{x1,y1,z0-e},{x0,y1,z0-e},cam,tile,fallback);
    m99TexturedQuad({x1,y0,z1+e},{x0,y0,z1+e},{x0,y1,z1+e},{x1,y1,z1+e},cam,tile,fallback);
    m99TexturedQuad({x0-e,y0,z1},{x0-e,y0,z0},{x0-e,y1,z0},{x0-e,y1,z1},cam,tile,fallback);
    m99TexturedQuad({x1+e,y0,z0},{x1+e,y0,z1},{x1+e,y1,z1},{x1+e,y1,z0},cam,tile,fallback);

    // Roof texture softens the plain flat-color roof.
    m99TexturedQuad({x0,c.y+sy+0.012f,z0},{x1,c.y+sy+0.012f,z0},
                    {x1,c.y+sy+0.012f,z1},{x0,c.y+sy+0.012f,z1},
                    cam,M99_ROOF,static_cast<unsigned>(RGBA8(105,108,110,255)));
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
        {x0,gM102WorldBaseY,z0},
        {x1,gM102WorldBaseY,z0},
        {x1,gM102WorldBaseY,z1},
        {x0,gM102WorldBaseY,z1},
        cam,color
    );
}

void drawExpandedGround(const Camera& cam) {
    // M83: larger neighborhood around the current physical-Vita test area.
    // Tiled instead of one huge quad so near-plane rejection does not remove
    // the whole ground when one corner is behind the camera.
    const unsigned grassA = static_cast<unsigned>(RGBA8(72, 94, 68, 255));
    const unsigned grassB = static_cast<unsigned>(RGBA8(78, 101, 72, 255));

    constexpr float TILE = 20.0f;
    // M89: larger geometry-only Vita test world.
    for (int z=-9; z<9; ++z) {
        for (int x=-9; x<9; ++x) {
            float x0 = x*TILE;
            float z0 = z*TILE;
            unsigned col = ((x+z)&1) ? grassA : grassB;
            groundTile(x0,z0,x0+TILE,z0+TILE,cam,col);
        }
    }
}

void roadX(float z, const Camera& cam) {
    const unsigned asphalt = static_cast<unsigned>(RGBA8(42,44,47,255));
    const unsigned line = static_cast<unsigned>(RGBA8(218,205,116,255));

    // M89 road from x -180 to +180.
    for (int i=0;i<18;i++) {
        float x0=-180.0f+i*20.0f;
        float x1=x0+20.0f;
        groundTile(x0,z-4.0f,x1,z+4.0f,cam,asphalt);
    }

    for (float x=-176.0f;x<176.0f;x+=10.0f) {
        groundTile(x,z-0.08f,x+5.0f,z+0.08f,cam,line);
    }
}

void roadZ(float x, const Camera& cam) {
    const unsigned asphalt = static_cast<unsigned>(RGBA8(42,44,47,255));
    const unsigned line = static_cast<unsigned>(RGBA8(218,205,116,255));

    // M89 road from z -180 to +180.
    for (int i=0;i<18;i++) {
        float z0=-180.0f+i*20.0f;
        float z1=z0+20.0f;
        groundTile(x-4.0f,z0,x+4.0f,z1,cam,asphalt);
    }

    for (float z=-176.0f;z<176.0f;z+=10.0f) {
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


// M92B compile fix: these M92 helpers are implemented later in this file.
// Forward declarations are required because cityBuilding() calls them first.
static void bevelBuilding(const Vec3& c,float sx,float sy,float sz,
                          const Camera& cam,unsigned front,unsigned side,unsigned top);
static void m92WindowsRoundedFacade(const Vec3& c,float sx,float sy,float sz,
                                    const Camera& cam);

void cityBuilding(const Vec3& c,float sx,float sy,float sz,
                  const Camera& cam,unsigned front,unsigned side,unsigned top) {
    // M92: beveled / faceted building replaces the old plain box silhouette.
    Vec3 wc=c;
    if(std::fabs(wc.y)<0.001f) wc.y=gM102WorldBaseY;
    bevelBuilding(wc,sx,sy,sz,cam,front,side,top);
    m99BuildingTextureOverlay(wc,sx,sy,sz,cam,
                              static_cast<unsigned>(std::fabs(c.x*13.0f+c.z*7.0f)));
    m92WindowsRoundedFacade(wc,sx,sy,sz,cam);
}

void drawExpandedBuildings(const Camera& cam) {
    // M87 separated-lot neighborhood: more buildings without returning to the old unsafe
    // stack-backed vita2d_draw_array path.
    struct B {
        float x,z,sx,sy,sz;
        unsigned front,side,top;
    };

    static const B buildings[] = {
        // M87: buildings are placed on separate lots with guaranteed gaps.
        // No two building footprints overlap and none sits on the roads.

        // Block A: between road Z=-22 and road Z=22, north of spawn road.
        {-58.0f, 25.0f, 10.0f,  8.0f, 10.0f, static_cast<unsigned>(RGBA8(150,142,145,255)), static_cast<unsigned>(RGBA8(110,104,108,255)), static_cast<unsigned>(RGBA8(180,174,177,255))},
        {-42.0f, 25.0f, 10.0f, 11.0f, 10.0f, static_cast<unsigned>(RGBA8(139,145,151,255)), static_cast<unsigned>(RGBA8(98,106,114,255)), static_cast<unsigned>(RGBA8(170,176,181,255))},
        { -8.0f, 25.0f, 10.0f,  7.0f, 10.0f, static_cast<unsigned>(RGBA8(164,143,123,255)), static_cast<unsigned>(RGBA8(119,99,83,255)), static_cast<unsigned>(RGBA8(190,169,147,255))},
        {  8.0f, 25.0f, 10.0f, 10.0f, 10.0f, static_cast<unsigned>(RGBA8(156,145,132,255)), static_cast<unsigned>(RGBA8(111,100,89,255)), static_cast<unsigned>(RGBA8(184,173,158,255))},
        { 42.0f, 25.0f, 10.0f, 12.0f, 10.0f, static_cast<unsigned>(RGBA8(153,150,139,255)), static_cast<unsigned>(RGBA8(108,106,96,255)), static_cast<unsigned>(RGBA8(180,177,165,255))},
        { 58.0f, 25.0f, 10.0f,  9.0f, 10.0f, static_cast<unsigned>(RGBA8(145,129,121,255)), static_cast<unsigned>(RGBA8(104,90,84,255)), static_cast<unsigned>(RGBA8(174,156,147,255))},

        // Block B: above the second east/west road at Z=52.
        {-58.0f, 70.0f, 11.0f, 17.0f, 11.0f, static_cast<unsigned>(RGBA8(130,139,145,255)), static_cast<unsigned>(RGBA8(91,99,106,255)), static_cast<unsigned>(RGBA8(160,169,175,255))},
        {-40.0f, 70.0f, 11.0f, 14.0f, 11.0f, static_cast<unsigned>(RGBA8(126,136,148,255)), static_cast<unsigned>(RGBA8(88,98,109,255)), static_cast<unsigned>(RGBA8(156,165,176,255))},
        { -8.0f, 70.0f, 11.0f, 18.0f, 11.0f, static_cast<unsigned>(RGBA8(126,137,149,255)), static_cast<unsigned>(RGBA8(88,98,110,255)), static_cast<unsigned>(RGBA8(157,167,178,255))},
        {  9.0f, 70.0f, 11.0f, 13.0f, 11.0f, static_cast<unsigned>(RGBA8(137,142,143,255)), static_cast<unsigned>(RGBA8(96,101,102,255)), static_cast<unsigned>(RGBA8(166,171,171,255))},
        { 40.0f, 70.0f, 11.0f, 16.0f, 11.0f, static_cast<unsigned>(RGBA8(122,132,137,255)), static_cast<unsigned>(RGBA8(86,95,100,255)), static_cast<unsigned>(RGBA8(153,163,168,255))},
        { 58.0f, 70.0f, 11.0f, 10.0f, 11.0f, static_cast<unsigned>(RGBA8(162,145,127,255)), static_cast<unsigned>(RGBA8(116,100,85,255)), static_cast<unsigned>(RGBA8(191,172,151,255))},

        // Block C: skyline row, also separated into individual lots.
        {-50.0f, 98.0f, 12.0f, 19.0f, 12.0f, static_cast<unsigned>(RGBA8(154,145,135,255)), static_cast<unsigned>(RGBA8(109,101,92,255)), static_cast<unsigned>(RGBA8(183,174,162,255))},
        {-30.0f, 98.0f, 12.0f, 22.0f, 12.0f, static_cast<unsigned>(RGBA8(119,130,142,255)), static_cast<unsigned>(RGBA8(83,93,104,255)), static_cast<unsigned>(RGBA8(150,160,171,255))},
        { -8.0f, 98.0f, 12.0f, 28.0f, 12.0f, static_cast<unsigned>(RGBA8(138,139,144,255)), static_cast<unsigned>(RGBA8(96,98,104,255)), static_cast<unsigned>(RGBA8(169,170,175,255))},
        { 12.0f, 98.0f, 12.0f, 24.0f, 12.0f, static_cast<unsigned>(RGBA8(128,137,146,255)), static_cast<unsigned>(RGBA8(89,98,107,255)), static_cast<unsigned>(RGBA8(159,168,177,255))},
        { 38.0f, 98.0f, 12.0f, 20.0f, 12.0f, static_cast<unsigned>(RGBA8(151,138,134,255)), static_cast<unsigned>(RGBA8(106,94,91,255)), static_cast<unsigned>(RGBA8(180,166,161,255))}
    };

    for (const B& b : buildings) {
        cityBuilding(
            {b.x,0.0f,b.z},
            b.sx,b.sy,b.sz,
            cam,b.front,b.side,b.top
        );
    }
}


void sidewalkStripX(float z,const Camera& cam) {
    const unsigned concrete=static_cast<unsigned>(RGBA8(150,151,148,255));
    for(int i=0;i<9;i++) {
        float x0=-80.0f+i*20.0f;
        float x1=x0+20.0f;
        groundTile(x0,z-5.4f,x1,z-4.2f,cam,concrete);
        groundTile(x0,z+4.2f,x1,z+5.4f,cam,concrete);
    }
}

void sidewalkStripZ(float x,const Camera& cam) {
    const unsigned concrete=static_cast<unsigned>(RGBA8(150,151,148,255));
    for(int i=0;i<10;i++) {
        float z0=-80.0f+i*20.0f;
        float z1=z0+20.0f;
        groundTile(x-5.4f,z0,x-4.2f,z1,cam,concrete);
        groundTile(x+4.2f,z0,x+5.4f,z1,cam,concrete);
    }
}

void streetLamp(float x,float z,const Camera& cam) {
    const unsigned pole=static_cast<unsigned>(RGBA8(66,68,70,255));
    const unsigned lamp=static_cast<unsigned>(RGBA8(238,211,126,255));
    boxPool({x,0.0f,z},0.16f,3.4f,0.16f,cam,pole,pole,pole);
    boxPool({x,3.35f,z},0.45f,0.16f,0.30f,cam,lamp,lamp,lamp);
}

void treeSimple(float x,float z,const Camera& cam) {
    const unsigned trunk=static_cast<unsigned>(RGBA8(92,68,45,255));
    const unsigned leaf=static_cast<unsigned>(RGBA8(54,104,58,255));
    boxPool({x,0.0f,z},0.30f,1.8f,0.30f,cam,trunk,trunk,trunk);
    boxPool({x,1.65f,z},1.25f,1.35f,1.25f,cam,leaf,leaf,leaf);
}

void benchSimple(float x,float z,const Camera& cam) {
    const unsigned wood=static_cast<unsigned>(RGBA8(116,82,53,255));
    const unsigned metal=static_cast<unsigned>(RGBA8(67,69,72,255));
    boxPool({x,0.35f,z},1.55f,0.18f,0.45f,cam,wood,wood,wood);
    boxPool({x-0.55f,0.0f,z},0.12f,0.35f,0.12f,cam,metal,metal,metal);
    boxPool({x+0.55f,0.0f,z},0.12f,0.35f,0.12f,cam,metal,metal,metal);
}

void parkingLot(float x0,float z0,float x1,float z1,const Camera& cam) {
    const unsigned asphalt=static_cast<unsigned>(RGBA8(49,51,53,255));
    const unsigned white=static_cast<unsigned>(RGBA8(205,205,197,255));
    groundTile(x0,z0,x1,z1,cam,asphalt);
    for(float x=x0+2.0f;x<x1-1.0f;x+=3.0f)
        groundTile(x,z0+0.8f,x+0.08f,z0+4.4f,cam,white);
}

void drawStreetDetails(const Camera& cam) {
    // Sidewalks around all four current roads.
    sidewalkStripX(7.0f,cam);
    sidewalkStripX(52.0f,cam);
    sidewalkStripZ(-22.0f,cam);
    sidewalkStripZ(22.0f,cam);

    // Lamps kept sparse for original PS Vita GPU/CPU budget.
    static const float lamps[][2]={
        {-62,13},{-42,13},{-10,13},{10,13},{42,13},{62,13},
        {-62,46},{-42,46},{-10,46},{10,46},{42,46},{62,46},
        {-28,24},{-28,40},{-28,66},{28,24},{28,40},{28,66}
    };
    for(const auto& p:lamps) streetLamp(p[0],p[1],cam);

    // Small green details between buildings.
    static const float trees[][2]={
        {-66,31},{-50,31},{-32,31},{-16,31},{16,31},{32,31},{50,31},{66,31},
        {-66,78},{-50,78},{-31,78},{16,78},{31,78},{50,78},{66,78}
    };
    for(const auto& p:trees) treeSimple(p[0],p[1],cam);

    benchSimple(-15.0f,34.0f,cam);
    benchSimple(15.0f,34.0f,cam);
    benchSimple(-15.0f,82.0f,cam);
    benchSimple(15.0f,82.0f,cam);

    // Two lightweight parking areas.
    parkingLot(-72.0f,38.0f,-52.0f,47.0f,cam);
    parkingLot(52.0f,38.0f,72.0f,47.0f,cam);
}


void drawOuterDistrictBuildings(const Camera& cam) {
    struct OB { float x,z,sx,sy,sz; unsigned c1,c2,c3; };
    static const OB outer[] = {
        {-145,-120,14,12,14, static_cast<unsigned>(RGBA8(142,137,132,255)), static_cast<unsigned>(RGBA8(100,95,90,255)), static_cast<unsigned>(RGBA8(170,165,159,255))},
        {-115,-120,14,16,14, static_cast<unsigned>(RGBA8(129,137,145,255)), static_cast<unsigned>(RGBA8(90,98,106,255)), static_cast<unsigned>(RGBA8(158,166,174,255))},
        { -75,-120,16,10,16, static_cast<unsigned>(RGBA8(157,144,128,255)), static_cast<unsigned>(RGBA8(111,99,85,255)), static_cast<unsigned>(RGBA8(187,172,152,255))},
        { -35,-120,15,18,15, static_cast<unsigned>(RGBA8(128,134,142,255)), static_cast<unsigned>(RGBA8(89,95,102,255)), static_cast<unsigned>(RGBA8(158,164,172,255))},
        {  35,-120,15,14,15, static_cast<unsigned>(RGBA8(149,141,134,255)), static_cast<unsigned>(RGBA8(105,98,92,255)), static_cast<unsigned>(RGBA8(177,169,161,255))},
        {  75,-120,16,21,16, static_cast<unsigned>(RGBA8(121,132,142,255)), static_cast<unsigned>(RGBA8(84,94,104,255)), static_cast<unsigned>(RGBA8(151,162,172,255))},
        { 115,-120,14,13,14, static_cast<unsigned>(RGBA8(160,146,131,255)), static_cast<unsigned>(RGBA8(114,101,88,255)), static_cast<unsigned>(RGBA8(190,174,156,255))},
        { 145,-120,14,17,14, static_cast<unsigned>(RGBA8(132,140,146,255)), static_cast<unsigned>(RGBA8(93,101,107,255)), static_cast<unsigned>(RGBA8(162,170,176,255))},

        {-145,-55,14,11,14, static_cast<unsigned>(RGBA8(151,143,137,255)), static_cast<unsigned>(RGBA8(106,99,94,255)), static_cast<unsigned>(RGBA8(181,173,166,255))},
        {-115,-55,16,20,16, static_cast<unsigned>(RGBA8(124,135,145,255)), static_cast<unsigned>(RGBA8(86,96,106,255)), static_cast<unsigned>(RGBA8(154,165,175,255))},
        { 115,-55,16,18,16, static_cast<unsigned>(RGBA8(137,143,147,255)), static_cast<unsigned>(RGBA8(96,102,106,255)), static_cast<unsigned>(RGBA8(166,172,176,255))},
        { 145,-55,14,12,14, static_cast<unsigned>(RGBA8(159,145,132,255)), static_cast<unsigned>(RGBA8(113,100,88,255)), static_cast<unsigned>(RGBA8(188,173,157,255))},

        {-145,15,14,15,14, static_cast<unsigned>(RGBA8(127,137,146,255)), static_cast<unsigned>(RGBA8(89,98,107,255)), static_cast<unsigned>(RGBA8(157,167,176,255))},
        {-115,15,14,10,14, static_cast<unsigned>(RGBA8(158,146,136,255)), static_cast<unsigned>(RGBA8(112,101,92,255)), static_cast<unsigned>(RGBA8(188,175,164,255))},
        { 115,15,14,22,14, static_cast<unsigned>(RGBA8(120,131,143,255)), static_cast<unsigned>(RGBA8(83,93,105,255)), static_cast<unsigned>(RGBA8(150,161,173,255))},
        { 145,15,14,14,14, static_cast<unsigned>(RGBA8(147,141,136,255)), static_cast<unsigned>(RGBA8(103,98,93,255)), static_cast<unsigned>(RGBA8(176,170,164,255))},

        {-145,90,16,18,16, static_cast<unsigned>(RGBA8(125,135,144,255)), static_cast<unsigned>(RGBA8(87,96,105,255)), static_cast<unsigned>(RGBA8(155,165,174,255))},
        {-115,90,14,12,14, static_cast<unsigned>(RGBA8(161,147,132,255)), static_cast<unsigned>(RGBA8(115,102,88,255)), static_cast<unsigned>(RGBA8(191,175,157,255))},
        { 115,90,15,17,15, static_cast<unsigned>(RGBA8(135,142,147,255)), static_cast<unsigned>(RGBA8(95,101,107,255)), static_cast<unsigned>(RGBA8(165,172,177,255))},
        { 145,90,14,11,14, static_cast<unsigned>(RGBA8(155,145,137,255)), static_cast<unsigned>(RGBA8(109,101,94,255)), static_cast<unsigned>(RGBA8(184,174,166,255))},

        {-145,145,14,24,14, static_cast<unsigned>(RGBA8(118,130,141,255)), static_cast<unsigned>(RGBA8(81,91,102,255)), static_cast<unsigned>(RGBA8(148,160,171,255))},
        {-105,145,16,15,16, static_cast<unsigned>(RGBA8(151,144,136,255)), static_cast<unsigned>(RGBA8(106,99,93,255)), static_cast<unsigned>(RGBA8(180,173,165,255))},
        { -60,145,15,20,15, static_cast<unsigned>(RGBA8(126,136,145,255)), static_cast<unsigned>(RGBA8(88,97,106,255)), static_cast<unsigned>(RGBA8(156,166,175,255))},
        { -15,145,14,13,14, static_cast<unsigned>(RGBA8(161,147,134,255)), static_cast<unsigned>(RGBA8(115,102,90,255)), static_cast<unsigned>(RGBA8(191,175,159,255))},
        {  30,145,15,23,15, static_cast<unsigned>(RGBA8(121,132,143,255)), static_cast<unsigned>(RGBA8(84,94,105,255)), static_cast<unsigned>(RGBA8(151,162,173,255))},
        {  75,145,16,14,16, static_cast<unsigned>(RGBA8(146,142,138,255)), static_cast<unsigned>(RGBA8(102,99,95,255)), static_cast<unsigned>(RGBA8(175,171,166,255))},
        { 120,145,15,19,15, static_cast<unsigned>(RGBA8(129,138,146,255)), static_cast<unsigned>(RGBA8(90,99,107,255)), static_cast<unsigned>(RGBA8(159,168,176,255))}
    };

    for(const OB& b: outer)
        cityBuilding({b.x,0.0f,b.z},b.sx,b.sy,b.sz,cam,b.c1,b.c2,b.c3);
}

void drawOuterRoadsAndDetails(const Camera& cam) {
    roadX(-95.0f,cam);
    roadX(-25.0f,cam);
    roadX(115.0f,cam);
    roadZ(-95.0f,cam);
    roadZ(95.0f,cam);

    sidewalkStripX(-95.0f,cam);
    sidewalkStripX(-25.0f,cam);
    sidewalkStripX(115.0f,cam);
    sidewalkStripZ(-95.0f,cam);
    sidewalkStripZ(95.0f,cam);

    static const float outerTrees[][2] = {
        {-165,-150},{-135,-150},{-95,-150},{-55,-150},{-15,-150},{25,-150},{65,-150},{105,-150},{145,-150},
        {-165,125},{-125,125},{-85,125},{-45,125},{-5,125},{35,125},{75,125},{115,125},{155,125}
    };
    for(const auto& p:outerTrees) treeSimple(p[0],p[1],cam);
}


// -----------------------------------------------------------------------------
// M90 FULL 120 KM x 120 KM STREAMED WORLD
// Whole map coordinates exist from -60,000m to +60,000m on X/Z.
// Original PS Vita only receives nearby 64m cells, never the whole map at once.
// No textures/images: geometry + solid colors only.
// -----------------------------------------------------------------------------
static constexpr float M90_WORLD_HALF = 60000.0f;
static constexpr float M90_CELL = 64.0f;
static constexpr int M90_RADIUS = 2;

static unsigned m90Hash(int x,int z) {
    unsigned h=static_cast<unsigned>(x)*0x8da6b343u;
    h^=static_cast<unsigned>(z)*0xd8163841u;
    h^=(h>>13); h*=0x85ebca6bu; return h^(h>>16);
}

static int m90Region(float x,float z) {
    // Lightweight canonical-region bands for staged Vita development.
    // Exact accepted map geography can be layered later without changing streaming.
    if(z < -42000.0f) return 0;                    // Mount Ridge / north
    if(z > 42000.0f && x > 10000.0f) return 1;    // Desert Plains
    if(z > 35000.0f && x < -15000.0f) return 2;   // Farmland / South Hills
    if(x > 43000.0f || x < -43000.0f) return 3;   // Coasts
    if(z > 25000.0f) return 4;                    // southern districts/port approach
    return 5;                                     // central Sight City / suburbs
}

static unsigned m90GroundColor(int region,unsigned h) {
    switch(region) {
        case 0: return static_cast<unsigned>(RGBA8(72,96,66,255));
        case 1: return static_cast<unsigned>(RGBA8(173,148,94,255));
        case 2: return static_cast<unsigned>(RGBA8(103,128,76,255));
        case 3: return static_cast<unsigned>(RGBA8(91,126,78,255));
        case 4: return static_cast<unsigned>(RGBA8(83,116,73,255));
        default:return ((h&7u)==0u)?static_cast<unsigned>(RGBA8(98,112,72,255)):
                                     static_cast<unsigned>(RGBA8(75,127,75,255));
    }
}


// M98 world-detail helpers ---------------------------------------------------
// M101B compile-order declarations ------------------------------------------
static void prismPool(const Vec3& c,float radius,float sy,int sides,
                      const Camera& cam,unsigned wallA,unsigned wallB,unsigned top);

struct M95PlacedBuilding {
    float x,z,sx,sz;
};
static bool m95CanPlace(const M95PlacedBuilding* placed,int count,
                        float x,float z,float sx,float sz) {
    const float gap=4.0f;
    const float ahx=sx*0.5f;
    const float ahz=sz*0.5f;

    for(int i=0;i<count;i++) {
        const float bhx=placed[i].sx*0.5f;
        const float bhz=placed[i].sz*0.5f;

        if(std::fabs(x-placed[i].x) < (ahx+bhx+gap) &&
           std::fabs(z-placed[i].z) < (ahz+bhz+gap)) {
            return false;
        }
    }
    return true;
}

static void m98Tree(float x,float z,const Camera& cam,unsigned seed) {
    const unsigned trunkA=static_cast<unsigned>(RGBA8(89,62,42,255));
    const unsigned trunkB=static_cast<unsigned>(RGBA8(65,44,30,255));
    const unsigned leafA=static_cast<unsigned>(RGBA8(40+(seed&13u),102,46,255));
    const unsigned leafB=static_cast<unsigned>(RGBA8(28,78+((seed>>4)&15u),35,255));
    const unsigned leafC=static_cast<unsigned>(RGBA8(58,121,53,255));

    // Thin trunk with two branch stubs.
    prismPool({x,gM102WorldBaseY,z},0.16f,3.05f,10,cam,trunkA,trunkB,trunkA);
    prismPool({x-0.22f,1.90f,z},0.07f,0.72f,8,cam,trunkA,trunkB,trunkA);
    prismPool({x+0.24f,2.02f,z-0.08f},0.07f,0.64f,8,cam,trunkA,trunkB,trunkA);

    // Irregular layered crown: deliberately not one cube/one sphere.
    prismPool({x,2.25f,z},0.74f,1.15f,12,cam,leafA,leafB,leafC);
    prismPool({x-0.62f,2.48f,z+0.18f},0.56f,0.88f,10,cam,leafB,leafA,leafC);
    prismPool({x+0.60f,2.55f,z-0.12f},0.58f,0.92f,10,cam,leafC,leafB,leafA);
    prismPool({x-0.18f,3.10f,z-0.34f},0.55f,0.88f,12,cam,leafA,leafC,leafB);
    prismPool({x+0.24f,3.28f,z+0.30f},0.48f,0.74f,10,cam,leafC,leafA,leafB);

    // Bark and foliage texture overlays.
    m99TexturedQuad({x-0.13f,gM102WorldBaseY,z-0.18f},{x+0.13f,gM102WorldBaseY,z-0.18f},
                    {x+0.13f,3.02f,z-0.18f},{x-0.13f,3.02f,z-0.18f},
                    cam,M99_BARK,trunkA);
    m99TexturedQuad({x-1.05f,2.22f,z-0.08f},{x+1.05f,2.22f,z-0.08f},
                    {x+1.05f,4.02f,z-0.08f},{x-1.05f,4.02f,z-0.08f},
                    cam,M99_LEAVES,leafA);
    m99TexturedQuad({x-0.08f,2.22f,z-1.03f},{x-0.08f,2.22f,z+1.03f},
                    {x-0.08f,4.02f,z+1.03f},{x-0.08f,4.02f,z-1.03f},
                    cam,M99_LEAVES,leafB);
}

static void m98StreetLamp(float x,float z,const Camera& cam) {
    const unsigned pole=static_cast<unsigned>(RGBA8(54,58,62,255));
    const unsigned metal=static_cast<unsigned>(RGBA8(73,77,82,255));
    const unsigned light=static_cast<unsigned>(RGBA8(255,224,142,255));
    const unsigned glow=static_cast<unsigned>(RGBA8(255,238,176,210));

    // Slim 8-sided pole instead of a blocky column.
    prismPool({x,gM102WorldBaseY,z},0.075f,3.45f,8,cam,pole,metal,pole);

    // Small horizontal arm and tapered-looking lamp head.
    boxPool({x+0.28f,3.30f,z},0.62f,0.10f,0.10f,cam,metal,pole,metal);
    prismPool({x+0.56f,3.20f,z},0.18f,0.18f,8,cam,metal,pole,metal);
    boxPool({x+0.56f,3.13f,z},0.34f,0.08f,0.22f,cam,light,glow,light);

    // Tiny warm halo geometry: cheap but reads much better on Vita screen.
    prismPool({x+0.56f,3.03f,z},0.22f,0.05f,8,cam,glow,glow,glow);
}

static void m98GrassClump(float x,float z,const Camera& cam,unsigned seed) {
    const unsigned g=static_cast<unsigned>(RGBA8(54,118+int(seed&12u),55,255));
    // Tiny crossed blades; cheap enough for a few per streamed cell.
    quadPool({x-0.05f,gM102WorldBaseY+0.01f,z},{x+0.05f,gM102WorldBaseY+0.01f,z},
             {x+0.03f,0.34f,z},{x-0.03f,0.34f,z},cam,g);
    quadPool({x,gM102WorldBaseY+0.01f,z-0.05f},{x,gM102WorldBaseY+0.01f,z+0.05f},
             {x,0.30f,z+0.03f},{x,0.30f,z-0.03f},cam,g);
}

static bool m98NearRoadX(int cx,float x) {
    if((cx%4)!=0) return false;
    const float rx=float(cx)*M90_CELL+32.0f;
    return std::fabs(x-rx)<8.0f;
}
static bool m98NearRoadZ(int cz,float z) {
    if((cz%4)!=0) return false;
    const float rz=float(cz)*M90_CELL+32.0f;
    return std::fabs(z-rz)<8.0f;
}

void drawM90Cell(int cx,int cz,const Camera& cam) {
    const float x0=cx*M90_CELL, z0=cz*M90_CELL;
    const float x1=x0+M90_CELL, z1=z0+M90_CELL;
    if(x1 < -M90_WORLD_HALF || x0 > M90_WORLD_HALF ||
       z1 < -M90_WORLD_HALF || z0 > M90_WORLD_HALF) return;

    const unsigned h=m90Hash(cx,cz);
    const int region=m90Region(x0+32.0f,z0+32.0f);
    m99GroundTile(x0,z0,x1,z1,cam,m99GroundTileForRegion(region),m90GroundColor(region,h));

    // Continuous sparse road grid across the entire map.
    const unsigned road=static_cast<unsigned>(RGBA8(46,47,49,255));
    const unsigned curb=static_cast<unsigned>(RGBA8(151,152,149,255));
    const unsigned line=static_cast<unsigned>(RGBA8(219,180,56,255));
    if((cx%4)==0) {
        float rx=x0+32.0f;
        m99GroundTile(rx-4.0f,z0,rx+4.0f,z1,cam,M99_ASPHALT,road);
        m99GroundTile(rx-5.0f,z0,rx-4.1f,z1,cam,M99_CONCRETE,curb);
        m99GroundTile(rx+4.1f,z0,rx+5.0f,z1,cam,M99_CONCRETE,curb);
        m99GroundTile(rx-7.0f,z0,rx-5.1f,z1,cam,M99_SIDEWALK,curb);
        m99GroundTile(rx+5.1f,z0,rx+7.0f,z1,cam,M99_SIDEWALK,curb);
        for(float zz=z0+4.0f;zz<z1;zz+=16.0f)
            groundTile(rx-0.35f,zz,rx+0.35f,zz+7.0f,cam,line);
    }
    if((cz%4)==0) {
        float rz=z0+32.0f;
        m99GroundTile(x0,rz-4.0f,x1,rz+4.0f,cam,M99_ASPHALT,road);
        m99GroundTile(x0,rz-5.0f,x1,rz-4.1f,cam,M99_CONCRETE,curb);
        m99GroundTile(x0,rz+4.1f,x1,rz+5.0f,cam,M99_CONCRETE,curb);
        m99GroundTile(x0,rz-7.0f,x1,rz-5.1f,cam,M99_SIDEWALK,curb);
        m99GroundTile(x0,rz+5.1f,x1,rz+7.0f,cam,M99_SIDEWALK,curb);
        for(float xx=x0+4.0f;xx<x1;xx+=16.0f)
            groundTile(xx,rz-0.35f,xx+7.0f,rz+0.35f,cam,line);
    }

    // M98 stable building lots. One layout system only; no duplicate M89 spawn block.
    unsigned density=0;
    if(region==5) density=2;
    else if(region==4 || region==3) density=((h>>4)&1u);
    else density=((h&7u)==0u)?1u:0u;

    static const float lotX[4]={12.0f,52.0f,12.0f,52.0f};
    static const float lotZ[4]={12.0f,12.0f,52.0f,52.0f};

    M95PlacedBuilding placed[4];
    int placedCount=0;

    for(unsigned i=0;i<density && i<4u;i++) {
        const unsigned q=m90Hash(cx*31+int(i)*17,cz*37+int(i)*23);
        float sx=8.0f+float((q>>16)%4u);
        float sz=8.0f+float((q>>20)%4u);
        float sy=(region==5?8.0f:5.0f)+float((q>>24)%(region==5?18u:7u));

        // Pick a fixed lot from cell hash. Very small deterministic jitter only.
        const int li=int((q+i)%4u);
        float bx=x0+lotX[li]+(float((q>>5)%5u)-2.0f)*0.45f;
        float bz=z0+lotZ[li]+(float((q>>9)%5u)-2.0f)*0.45f;

        // Never put a building footprint on the cell's road corridor.
        if(m98NearRoadX(cx,bx)) bx = (bx < x0+32.0f) ? x0+12.0f : x0+52.0f;
        if(m98NearRoadZ(cz,bz)) bz = (bz < z0+32.0f) ? z0+12.0f : z0+52.0f;

        if(!m95CanPlace(placed,placedCount,bx,bz,sx,sz)) continue;
        placed[placedCount++]={bx,bz,sx,sz};

        const unsigned c1=static_cast<unsigned>(RGBA8(132+(q&23u),130+((q>>5)&23u),126+((q>>10)&23u),255));
        const unsigned c2=static_cast<unsigned>(RGBA8(91+(q&18u),91+((q>>5)&18u),91+((q>>10)&18u),255));
        const unsigned c3=static_cast<unsigned>(RGBA8(165+(q&18u),162+((q>>5)&18u),158+((q>>10)&18u),255));
        cityBuilding({bx,0.0f,bz},sx,sy,sz,cam,c1,c2,c3);
    }

    // Better but capped vegetation / grass detail.
    if(region!=1) {
        for(unsigned i=0;i<3u;i++) {
            const unsigned q=m90Hash(cx*73+int(i)*11,cz*79+int(i)*13);
            float gx=x0+7.0f+float(q%50u);
            float gz=z0+7.0f+float((q>>8)%50u);
            if(!m98NearRoadX(cx,gx) && !m98NearRoadZ(cz,gz))
                m98GrassClump(gx,gz,cam,q);
        }

        if((h&3u)==0u) {
            float tx=x0+10.0f+float((h>>9)%44u);
            float tz=z0+10.0f+float((h>>15)%44u);
            if(!m98NearRoadX(cx,tx) && !m98NearRoadZ(cz,tz))
                m98Tree(tx,tz,cam,h);
        }
    }

    // Lamps follow roads at fixed world positions.
    if((cx%4)==0) {
        const float rx=x0+32.0f;
        m98StreetLamp(rx-6.0f,z0+14.0f,cam);
        m98StreetLamp(rx+6.0f,z0+50.0f,cam);
    }
    if((cz%4)==0) {
        const float rz=z0+32.0f;
        m98StreetLamp(x0+14.0f,rz-6.0f,cam);
        m98StreetLamp(x0+50.0f,rz+6.0f,cam);
    }
}

void drawM90FullMapStream(const Camera& cam) {
    int ccx=static_cast<int>(std::floor(cam.position.x/M90_CELL));
    int ccz=static_cast<int>(std::floor(cam.position.z/M90_CELL));
    for(int dz=-M90_RADIUS;dz<=M90_RADIUS;dz++)
        for(int dx=-M90_RADIUS;dx<=M90_RADIUS;dx++)
            drawM90Cell(ccx+dx,ccz+dz,cam);
}



// -----------------------------------------------------------------------------
// M92 REAL-3D WORLD / CANONICAL MAP PASS
// Removes the obvious cube-only look by using beveled / multi-sided geometry,
// sloped terrain and region-aware streamed scenery.
// -----------------------------------------------------------------------------
enum class M92Region {
    MountRidge, NorthCountryside, PineValley, EastCoast,
    Riverside, Lakeside, Downtown, WestSuburbs, EastSuburbs,
    Industrial, Airport, Farmland, WestCoast, SouthHills,
    DesertPlains, Port
};

static M92Region m92Region(float x,float z) {
    // Canonical 120x120 km Sight City layout, normalized to map coordinates.
    const float nx=(x+60000.0f)/120000.0f;
    const float nz=(z+60000.0f)/120000.0f;

    if(nz < 0.18f) {
        if(nx < 0.32f) return M92Region::MountRidge;
        if(nx < 0.67f) return M92Region::NorthCountryside;
        return M92Region::PineValley;
    }
    if(nz < 0.38f) {
        if(nx < 0.20f) return M92Region::Riverside;
        if(nx < 0.63f) return M92Region::Lakeside;
        return M92Region::EastCoast;
    }
    if(nz < 0.64f) {
        if(nx < 0.30f) return M92Region::WestSuburbs;
        if(nx < 0.56f) return M92Region::Downtown;
        if(nx < 0.78f) return M92Region::EastSuburbs;
        return M92Region::Industrial;
    }
    if(nz < 0.82f) {
        if(nx < 0.32f) return M92Region::Farmland;
        if(nx < 0.58f) return M92Region::SouthHills;
        if(nx < 0.78f) return M92Region::Airport;
        return M92Region::DesertPlains;
    }
    if(nx < 0.36f) return M92Region::WestCoast;
    if(nx < 0.68f) return M92Region::SouthHills;
    if(nx < 0.83f) return M92Region::DesertPlains;
    return M92Region::Port;
}

static float m92MountainHeight(float x,float z) {
    const M92Region r=m92Region(x,z);
    float h=0.0f;

    // Large mountain masses: Mount Ridge reaches roughly 1,800 m in world data.
    if(r==M92Region::MountRidge) {
        const float cx=-43000.0f, cz=-45500.0f;
        const float dx=(x-cx)/15500.0f, dz=(z-cz)/14500.0f;
        const float d=dx*dx+dz*dz;
        if(d<1.0f) h += (1.0f-d)*1800.0f;
        h += 120.0f*std::sin(x*0.0011f)*std::cos(z*0.0013f);
    } else if(r==M92Region::SouthHills) {
        const float cx=0.0f, cz=36000.0f;
        const float dx=(x-cx)/23000.0f, dz=(z-cz)/13000.0f;
        const float d=dx*dx+dz*dz;
        if(d<1.0f) h += (1.0f-d)*700.0f;
        h += 55.0f*std::sin(x*0.0015f+z*0.0007f);
    } else if(r==M92Region::PineValley || r==M92Region::NorthCountryside) {
        h += 45.0f + 35.0f*std::sin(x*0.0012f)*std::sin(z*0.0010f);
    }
    return h<0.0f?0.0f:h;
}

static unsigned m92TerrainColor(M92Region r,float h) {
    if(r==M92Region::DesertPlains) return static_cast<unsigned>(RGBA8(168,145,95,255));
    if(r==M92Region::WestCoast || r==M92Region::EastCoast) return static_cast<unsigned>(RGBA8(112,132,83,255));
    if(r==M92Region::Industrial || r==M92Region::Port || r==M92Region::Airport)
        return static_cast<unsigned>(RGBA8(100,103,98,255));
    if(r==M92Region::MountRidge && h>1100.0f) return static_cast<unsigned>(RGBA8(210,214,213,255));
    if(r==M92Region::MountRidge && h>500.0f) return static_cast<unsigned>(RGBA8(103,106,100,255));
    return static_cast<unsigned>(RGBA8(75,112,68,255));
}

static void prismPool(const Vec3& c,float radius,float sy,int sides,
                      const Camera& cam,unsigned wallA,unsigned wallB,unsigned top) {
    if(sides<5) sides=5;
    if(sides>12) sides=12;
    Vec3 ring0[12], ring1[12];
    for(int i=0;i<sides;i++) {
        const float a=6.283185307f*float(i)/float(sides);
        ring0[i]={c.x+std::cos(a)*radius,c.y,c.z+std::sin(a)*radius};
        ring1[i]={ring0[i].x,c.y+sy,ring0[i].z};
    }
    for(int i=0;i<sides;i++) {
        const int j=(i+1)%sides;
        quadPool(ring0[i],ring0[j],ring1[j],ring1[i],cam,(i&1)?wallA:wallB);
        quadPool({c.x,c.y+sy,c.z},ring1[i],ring1[j],{c.x,c.y+sy,c.z},cam,top);
    }
}

static void bevelBuilding(const Vec3& c,float sx,float sy,float sz,
                          const Camera& cam,unsigned front,unsigned side,unsigned top) {
    // Beveled octagonal footprint instead of a rectangular Minecraft-like box.
    const float bx=sx*0.16f, bz=sz*0.16f;
    const float x0=c.x-sx*0.5f, x1=c.x+sx*0.5f;
    const float z0=c.z-sz*0.5f, z1=c.z+sz*0.5f;
    Vec3 p[8]={
        {x0+bx,c.y,z0},{x1-bx,c.y,z0},{x1,c.y,z0+bz},{x1,c.y,z1-bz},
        {x1-bx,c.y,z1},{x0+bx,c.y,z1},{x0,c.y,z1-bz},{x0,c.y,z0+bz}
    };
    Vec3 q[8];
    for(int i=0;i<8;i++) q[i]={p[i].x,c.y+sy,p[i].z};

    for(int i=0;i<8;i++) {
        const int j=(i+1)&7;
        quadPool(p[i],p[j],q[j],q[i],cam,(i&1)?side:front);
    }
    for(int i=1;i<7;i++)
        quadPool(q[0],q[i],q[i+1],q[0],cam,top);

    // Sloped / stepped roof silhouette.
    if(sy>8.0f) {
        const unsigned roofDark=static_cast<unsigned>(RGBA8(75,78,81,255));
        prismPool({c.x,c.y+sy,c.z},sx*0.23f,0.55f,8,cam,roofDark,top,top);
    }
}

static void m92WindowsRoundedFacade(const Vec3& c,float sx,float sy,float sz,const Camera& cam) {
    const unsigned glass=static_cast<unsigned>(RGBA8(72,111,133,255));
    const unsigned frame=static_cast<unsigned>(RGBA8(48,52,55,255));
    const float z=c.z-sz*0.5f-0.025f;
    const int floors=(int)(sy/2.8f);
    for(int f=0;f<floors && f<8;f++) {
        const float y=c.y+1.4f+f*2.5f;
        for(int w=-1;w<=1;w++) {
            const float x=c.x+w*sx*0.24f;
            quadPool({x-0.55f,y,z},{x+0.55f,y,z},{x+0.55f,y+0.75f,z},{x-0.55f,y+0.75f,z},cam,glass);
            quadPool({x-0.62f,y-0.06f,z-0.01f},{x+0.62f,y-0.06f,z-0.01f},
                     {x+0.62f,y,z-0.01f},{x-0.62f,y,z-0.01f},cam,frame);
        }
    }
}

static void m92ModernBuilding(const Vec3& c,float sx,float sy,float sz,const Camera& cam,
                              unsigned a,unsigned b,unsigned top) {
    bevelBuilding(c,sx,sy,sz,cam,a,b,top);
    m92WindowsRoundedFacade(c,sx,sy,sz,cam);

    // Recessed entrance + canopy.
    const unsigned door=static_cast<unsigned>(RGBA8(52,61,66,255));
    const unsigned trim=static_cast<unsigned>(RGBA8(96,101,104,255));
    quadPool({c.x-0.9f,c.y,c.z-sz*0.5f-0.04f},{c.x+0.9f,c.y,c.z-sz*0.5f-0.04f},
             {c.x+0.9f,c.y+2.2f,c.z-sz*0.5f-0.04f},{c.x-0.9f,c.y+2.2f,c.z-sz*0.5f-0.04f},cam,door);
    quadPool({c.x-1.7f,c.y+2.25f,c.z-sz*0.5f-0.55f},{c.x+1.7f,c.y+2.25f,c.z-sz*0.5f-0.55f},
             {c.x+1.7f,c.y+2.25f,c.z-sz*0.5f+0.15f},{c.x-1.7f,c.y+2.25f,c.z-sz*0.5f+0.15f},cam,trim);
}

static void m92TerrainPatch(float x0,float z0,float size,const Camera& cam) {
    const int N=4;
    const float step=size/float(N);
    for(int iz=0;iz<N;iz++) for(int ix=0;ix<N;ix++) {
        const float xa=x0+ix*step, xb=xa+step;
        const float za=z0+iz*step, zb=za+step;
        const float h00=m92MountainHeight(xa,za);
        const float h10=m92MountainHeight(xb,za);
        const float h11=m92MountainHeight(xb,zb);
        const float h01=m92MountainHeight(xa,zb);
        const M92Region r=m92Region(xa+step*0.5f,za+step*0.5f);
        const unsigned col=m92TerrainColor(r,(h00+h10+h11+h01)*0.25f);
        quadPool({xa,h00,za},{xb,h10,za},{xb,h11,zb},{xa,h01,zb},cam,col);
    }
}

static void m92MountainsNearCamera(const Camera& cam) {
    // 5x5 terrain patches around camera; keeps mountain geometry streamed.
    const float P=128.0f;
    const int cx=(int)std::floor(cam.position.x/P);
    const int cz=(int)std::floor(cam.position.z/P);
    for(int dz=-2;dz<=2;dz++) for(int dx=-2;dx<=2;dx++) {
        const float x0=float(cx+dx)*P;
        const float z0=float(cz+dz)*P;
        m92TerrainPatch(x0,z0,P,cam);
    }
}

static void m92CanonicalLandmarks(const Camera& cam) {
    // Region-specific silhouettes make the full map read as the planned map.
    const M92Region r=m92Region(cam.position.x,cam.position.z);
    const float x=cam.position.x, z=cam.position.z;

    if(r==M92Region::Downtown) {
        m92ModernBuilding({x-22,0,z+42},15,32,15,cam,
            static_cast<unsigned>(RGBA8(137,142,147,255)),
            static_cast<unsigned>(RGBA8(97,103,109,255)),
            static_cast<unsigned>(RGBA8(170,175,179,255)));
        m92ModernBuilding({x+24,0,z+48},18,45,18,cam,
            static_cast<unsigned>(RGBA8(129,136,143,255)),
            static_cast<unsigned>(RGBA8(90,98,106,255)),
            static_cast<unsigned>(RGBA8(159,166,173,255)));
    } else if(r==M92Region::Industrial || r==M92Region::Port) {
        prismPool({x-18,0,z+38},5.0f,13.0f,10,cam,
            static_cast<unsigned>(RGBA8(112,115,115,255)),
            static_cast<unsigned>(RGBA8(82,85,86,255)),
            static_cast<unsigned>(RGBA8(145,146,144,255)));
        prismPool({x+18,0,z+45},6.0f,10.0f,10,cam,
            static_cast<unsigned>(RGBA8(126,120,105,255)),
            static_cast<unsigned>(RGBA8(91,87,77,255)),
            static_cast<unsigned>(RGBA8(151,145,129,255)));
    } else if(r==M92Region::WestSuburbs || r==M92Region::EastSuburbs) {
        // Low houses with pitched roofs made from triangles/quads, not cubes alone.
        const unsigned wall=static_cast<unsigned>(RGBA8(172,160,145,255));
        const unsigned side=static_cast<unsigned>(RGBA8(132,120,108,255));
        const unsigned roof=static_cast<unsigned>(RGBA8(92,75,65,255));
        bevelBuilding({x-15,0,z+28},10,5.5f,12,cam,wall,side,wall);
        quadPool({x-20,5.5f,z+22},{x-10,5.5f,z+22},{x-15,8.2f,z+28},{x-15,8.2f,z+28},cam,roof);
        quadPool({x-10,5.5f,z+34},{x-20,5.5f,z+34},{x-15,8.2f,z+28},{x-15,8.2f,z+28},cam,roof);
    }
}


// -----------------------------------------------------------------------------
// M91 VISUAL + NPC AI PASS
// Original Vita target: geometry detail is distance-limited and NPC count capped.
// -----------------------------------------------------------------------------
struct M91Npc {
    Vec3 p;
    Vec3 goal;
    float speed;
    float heading;
    float walkPhase;
    float idleTimer;
    unsigned seed;
};

static M91Npc gM91Npcs[12];
static bool gM91NpcInit=false;

static float m91Dist2XZ(const Vec3& a,const Vec3& b) {
    const float dx=a.x-b.x, dz=a.z-b.z;
    return dx*dx+dz*dz;
}

static void m91InitNpcs(const Camera& cam) {
    (void)cam;
    for(int i=0;i<12;i++) {
        const float a=float(i)*0.5235987f;
        gM91Npcs[i].p={std::cos(a)*(18.0f+float(i%4)*5.0f),0.0f,
                       std::sin(a)*(18.0f+float(i%4)*5.0f)};
        gM91Npcs[i].goal={gM91Npcs[i].p.x+float((i%3)-1)*18.0f,0.0f,
                          gM91Npcs[i].p.z+float(((i+1)%3)-1)*18.0f};
        gM91Npcs[i].speed=0.70f+0.09f*float(i%5);
        gM91Npcs[i].heading=a;
        gM91Npcs[i].walkPhase=float(i)*0.45f;
        gM91Npcs[i].idleTimer=0.0f;
        gM91Npcs[i].seed=0x1234u+unsigned(i)*977u;
    }
    gM91NpcInit=true;
}

static void m91UpdateNpcs(const Camera& cam,float dt) {
    if(!gM91NpcInit) m91InitNpcs(cam);
    if(dt<0.0f) dt=0.0f;
    if(dt>0.05f) dt=0.05f;

    for(int i=0;i<12;i++) {
        M91Npc& n=gM91Npcs[i];

        if(n.idleTimer>0.0f) {
            n.idleTimer-=dt;
            continue;
        }

        float dx=n.goal.x-n.p.x, dz=n.goal.z-n.p.z;
        float d2=dx*dx+dz*dz;
        if(d2<2.0f) {
            n.seed=n.seed*1664525u+1013904223u;
            float ox=float(int((n.seed>>8)&31u)-15);
            n.seed=n.seed*1664525u+1013904223u;
            float oz=float(int((n.seed>>8)&31u)-15);
            n.goal={n.p.x+ox,0.0f,n.p.z+oz};
            if((n.seed&3u)==0u) n.idleTimer=0.7f+float((n.seed>>5)&7u)*0.15f;
        } else {
            float inv=1.0f/std::sqrt(d2);
            n.heading=std::atan2(dx,dz);
            n.p.x+=dx*inv*n.speed*dt;
            n.p.z+=dz*inv*n.speed*dt;
            n.walkPhase+=dt*(4.0f+n.speed*2.0f);
        }
    }
}

static void m91DrawNpc(const M91Npc& n,const Camera& cam) {
    const unsigned jeans=static_cast<unsigned>(RGBA8(45+int(n.seed&15u),55,75,255));
    const unsigned shirt=static_cast<unsigned>(RGBA8(85+int((n.seed>>4)&55u),50+int((n.seed>>10)&35u),65+int((n.seed>>15)&30u),255));
    const unsigned skin=static_cast<unsigned>(RGBA8(188,149,116,255));
    const unsigned hair=static_cast<unsigned>(RGBA8(45,37,33,255));
    const unsigned shoes=static_cast<unsigned>(RGBA8(28,29,31,255));

    const float swing=std::sin(n.walkPhase)*0.10f;
    const float sn=std::sin(n.heading), cs=std::cos(n.heading);

    auto wp=[&](float side,float up,float fwd)->Vec3 {
        return {n.p.x+cs*side+sn*fwd,gM102WorldBaseY+n.p.y+up,n.p.z-sn*side+cs*fwd};
    };

    // Rounded low-poly body: no cube torso/head.
    prismPool(wp(-0.15f,0.00f, swing),0.12f,0.76f,8,cam,jeans,jeans,jeans);
    prismPool(wp( 0.15f,0.00f,-swing),0.12f,0.76f,8,cam,jeans,jeans,jeans);
    prismPool(wp(-0.15f,0.00f, swing+0.07f),0.15f,0.16f,8,cam,shoes,shoes,shoes);
    prismPool(wp( 0.15f,0.00f,-swing+0.07f),0.15f,0.16f,8,cam,shoes,shoes,shoes);

    prismPool(wp(0.0f,0.72f,0.0f),0.34f,0.76f,10,cam,shirt,shirt,shirt);
    prismPool(wp(-0.39f,0.79f,-swing),0.075f,0.64f,8,cam,skin,skin,skin);
    prismPool(wp( 0.39f,0.79f, swing),0.075f,0.64f,8,cam,skin,skin,skin);

    prismPool(wp(0.0f,1.46f,0.0f),0.23f,0.42f,12,cam,skin,skin,skin);
    prismPool(wp(0.0f,1.80f,-0.01f),0.235f,0.08f,12,cam,hair,hair,hair);
}

static void m91DrawNpcs(const Camera& cam) {
    for(int i=0;i<12;i++)
        if(m91Dist2XZ(gM91Npcs[i].p,cam.position)<72.0f*72.0f)
            m91DrawNpc(gM91Npcs[i],cam);
}

static void m91BuildingDetail(const Vec3& p,float sx,float sy,float sz,const Camera& cam) {
    // Extra true-3D geometry: roof lip, doorway, window strips, small awning.
    const unsigned trim=static_cast<unsigned>(RGBA8(82,86,90,255));
    const unsigned glass=static_cast<unsigned>(RGBA8(92,132,151,255));
    const unsigned door=static_cast<unsigned>(RGBA8(58,48,42,255));
    boxPool({p.x,p.y+sy,p.z},sx+0.45f,0.28f,sz+0.45f,cam,trim,trim,trim);
    boxPool({p.x,p.y+1.05f,p.z-sz*0.51f},1.15f,2.10f,0.12f,cam,door,door,door);
    boxPool({p.x,p.y+2.35f,p.z-sz*0.53f},2.1f,0.18f,0.65f,cam,trim,trim,trim);

    if(sy>7.0f) {
        for(float y=2.2f;y<sy-1.0f;y+=2.4f) {
            boxPool({p.x-sx*0.24f,p.y+y,p.z-sz*0.515f},1.15f,0.72f,0.10f,cam,glass,glass,glass);
            boxPool({p.x+sx*0.24f,p.y+y,p.z-sz*0.515f},1.15f,0.72f,0.10f,cam,glass,glass,glass);
        }
    }
}

static void m91DetailedBlock(const Camera& cam) {
    // High-detail demonstration buildings close to the known spawn.
    const unsigned wallA=static_cast<unsigned>(RGBA8(151,143,132,255));
    const unsigned wallB=static_cast<unsigned>(RGBA8(112,105,96,255));
    const unsigned roof=static_cast<unsigned>(RGBA8(179,169,154,255));

    const Vec3 a={-18.0f,0.0f,-12.0f};
    const Vec3 b={18.0f,0.0f,-12.0f};
    cityBuilding(a,12.0f,12.0f,14.0f,cam,wallA,wallB,roof);
    cityBuilding(b,14.0f,18.0f,13.0f,cam,wallA,wallB,roof);
    m91BuildingDetail(a,12.0f,12.0f,14.0f,cam);
    m91BuildingDetail(b,14.0f,18.0f,13.0f,cam);
}

static void m91Frame(const Camera& cam) {
    // Fixed prototype step avoids touching the already-confirmed M81/M84 input loop.
    // Later this can receive the game's real dt.
    m91UpdateNpcs(cam,1.0f/30.0f);
    m91DrawNpcs(cam);
    if(std::fabs(cam.position.x)<90.0f && std::fabs(cam.position.z)<90.0f)
        m91DetailedBlock(cam);
}


// M96 fixed map scenery -------------------------------------------------------
// These mountains are VISUAL landmarks anchored to fixed world coordinates.
// They never use cam.position as their world position, so they cannot follow Dash.
// Walkable ground remains the collision/world ground; this avoids visual ground
// lifting under the player and making Dash look like he is floating.
static void m96MountainPeak(const Vec3& base,float radius,float height,const Camera& cam,
                            unsigned low,unsigned high) {
    const int sides=10;
    Vec3 ring[sides];
    for(int i=0;i<sides;i++) {
        const float a=6.28318530718f*float(i)/float(sides);
        ring[i]={base.x+std::cos(a)*radius,base.y+gM102WorldBaseY,base.z+std::sin(a)*radius};
    }
    const Vec3 top={base.x,base.y+gM102WorldBaseY+height,base.z};
    for(int i=0;i<sides;i++) {
        const int j=(i+1)%sides;
        const P2 pa = projectSafe(ring[i], cam);
        const P2 pb = projectSafe(ring[j], cam);
        const P2 pc = projectSafe(top, cam);
        triPool(pa, pb, pc, (i&1) ? low : high);
    }
}

static void m96FixedMountainScenery(const Camera& cam) {
    const unsigned rockA=static_cast<unsigned>(RGBA8(88,92,86,255));
    const unsigned rockB=static_cast<unsigned>(RGBA8(108,111,103,255));
    const unsigned snowA=static_cast<unsigned>(RGBA8(204,210,210,255));
    const unsigned hillA=static_cast<unsigned>(RGBA8(82,106,70,255));

    // Mount Ridge: expanded fixed range in the canonical north-west.
    // Draw only when reasonably near the range to protect original Vita FPS.
    const float dx=cam.position.x+43000.0f;
    const float dz=cam.position.z+45500.0f;
    if(dx*dx+dz*dz < 18000.0f*18000.0f) {
        m96MountainPeak({-50000.0f,0.0f,-49000.0f},3600.0f,1450.0f,cam,rockA,rockB);
        m96MountainPeak({-45500.0f,0.0f,-47000.0f},4300.0f,1800.0f,cam,rockA,snowA);
        m96MountainPeak({-40500.0f,0.0f,-46500.0f},3500.0f,1320.0f,cam,rockA,rockB);
        m96MountainPeak({-36500.0f,0.0f,-44000.0f},2900.0f,1050.0f,cam,rockA,rockB);
        m96MountainPeak({-48500.0f,0.0f,-42000.0f},3000.0f,980.0f,cam,rockA,rockB);
        m96MountainPeak({-42500.0f,0.0f,-41000.0f},2600.0f,850.0f,cam,rockA,rockB);
    }

    // South Hills remains a lower fixed range.
    const float sx=cam.position.x;
    const float sz=cam.position.z-36000.0f;
    if(sx*sx+sz*sz < 15000.0f*15000.0f) {
        m96MountainPeak({-6500.0f,0.0f,35000.0f},3000.0f,620.0f,cam,hillA,rockB);
        m96MountainPeak({0.0f,0.0f,37500.0f},3600.0f,700.0f,cam,hillA,rockB);
        m96MountainPeak({6500.0f,0.0f,35500.0f},2800.0f,540.0f,cam,hillA,rockB);
    }
}


// M97 visible Dash -----------------------------------------------------------
// Low-poly third-person player model. Uses existing pool-safe geometry helpers.
// Feet are anchored to Player::position.y, which M96 locks to collision ground.
static void m97DrawDash(const Player& player,const Camera& cam) {
    if(player.inVehicle) return;

    const float x=player.position.x;
    const float y=player.position.y;
    const float z=player.position.z;
    const float a=player.heading;
    const float sn=std::sin(a), cs=std::cos(a);

    const float speed=std::sqrt(player.velocity.x*player.velocity.x+
                                player.velocity.z*player.velocity.z);
    static float walkPhase=0.0f;
    if(speed>0.15f) walkPhase += 0.16f + std::min(speed,6.5f)*0.025f;
    const float swing=(speed>0.15f)?std::sin(walkPhase)*0.12f:0.0f;

    const unsigned pants=static_cast<unsigned>(RGBA8(34,39,48,255));
    const unsigned shoes=static_cast<unsigned>(RGBA8(20,21,24,255));
    const unsigned jacket=static_cast<unsigned>(RGBA8(48,57,70,255));
    const unsigned shirt=static_cast<unsigned>(RGBA8(171,47,47,255));
    const unsigned skin=static_cast<unsigned>(RGBA8(188,143,112,255));
    const unsigned hair=static_cast<unsigned>(RGBA8(37,31,29,255));

    auto wp=[&](float side,float up,float fwd)->Vec3 {
        return {x+cs*side+sn*fwd,y+up,z-sn*side+cs*fwd};
    };

    // More human silhouette: rounded 8/10/12-sided components.
    prismPool(wp(-0.17f,0.00f, swing),0.13f,0.86f,8,cam,pants,pants,pants);
    prismPool(wp( 0.17f,0.00f,-swing),0.13f,0.86f,8,cam,pants,pants,pants);
    prismPool(wp(-0.17f,0.00f, swing+0.09f),0.16f,0.18f,8,cam,shoes,shoes,shoes);
    prismPool(wp( 0.17f,0.00f,-swing+0.09f),0.16f,0.18f,8,cam,shoes,shoes,shoes);

    prismPool(wp(0.0f,0.82f,0.0f),0.38f,0.78f,10,cam,jacket,jacket,jacket);
    // red shirt visible in front
    m99TexturedQuad(wp(-0.15f,0.96f,0.37f),wp(0.15f,0.96f,0.37f),
                    wp(0.15f,1.42f,0.37f),wp(-0.15f,1.42f,0.37f),
                    cam,M99_WALL,shirt);

    prismPool(wp(-0.46f,0.88f,-swing),0.085f,0.70f,8,cam,jacket,jacket,jacket);
    prismPool(wp( 0.46f,0.88f, swing),0.085f,0.70f,8,cam,jacket,jacket,jacket);
    prismPool(wp(-0.46f,0.86f,-swing),0.09f,0.13f,8,cam,skin,skin,skin);
    prismPool(wp( 0.46f,0.86f, swing),0.09f,0.13f,8,cam,skin,skin,skin);

    prismPool(wp(0.0f,1.55f,0.0f),0.25f,0.46f,12,cam,skin,skin,skin);
    prismPool(wp(0.0f,1.92f,-0.01f),0.26f,0.09f,12,cam,hair,hair,hair);
}

void drawTestCity(const Camera& cam) {
    // Full 120 x 120 km world through streaming.
    drawM90FullMapStream(cam);

    // M98: the old M89 block is NOT drawn on top of M90 anymore.
    // It caused duplicated roads/buildings and visible spawn/overlap glitches.
    // M90 is now the single world-layout source around the player.
    m91Frame(cam);

    // M96: REMOVED camera-following terrain/landmarks.
    // They made scenery appear to move with the camera and the visual terrain
    // did not match gameplay collision height.
    m96FixedMountainScenery(cam);
}

} // namespace


// M94: coordinate HUD is independent from the Dev Menu panel.
// It stays in the upper-right corner and can be toggled from Dev Menu.
static vita2d_pgf* gM94DebugFont = nullptr;

static void m94DrawCoordinatesHud(const Player& player) {
    if(!DevDebugState::coordinatesHudEnabled() || !gM94DebugFont) return;

    char line1[96];
    char line2[96];
    std::snprintf(line1,sizeof(line1),"X %.2f   Y %.2f",player.position.x,player.position.y);
    std::snprintf(line2,sizeof(line2),"Z %.2f",player.position.z);

    const float scale = 0.72f;
    const int pad = 10;
    const int boxW = 255;
    const int boxH = 58;
    const int x = 960 - boxW - 14;
    const int y = 14;

    vita2d_draw_rectangle((float)x,(float)y,(float)boxW,(float)boxH,RGBA8(0,0,0,185));
    vita2d_draw_rectangle((float)x,(float)y,(float)boxW,2.0f,RGBA8(210,55,55,255));
    vita2d_pgf_draw_text(gM94DebugFont,x+pad,y+24,RGBA8(255,255,255,255),scale,line1);
    vita2d_pgf_draw_text(gM94DebugFont,x+pad,y+47,RGBA8(220,230,238,255),scale,line2);
}


// M95 -----------------------------------------------------------------------
// Stable world anchoring for streamed procedural content.
// IMPORTANT: object placement may depend on world cell coordinates + fixed seed
// only. Never use camera/player position to choose an object's world position.
static unsigned m95HashCell(int cx,int cz,unsigned salt) {
    unsigned x = (unsigned)cx * 0x8da6b343u;
    unsigned z = (unsigned)cz * 0xd8163841u;
    unsigned h = x ^ z ^ salt ^ 0xcb1ab31fu;
    h ^= h >> 16; h *= 0x7feb352du;
    h ^= h >> 15; h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}
static float m95Rand01(unsigned h) {
    return (float)(h & 0x00ffffffu) / 16777215.0f;
}
static float m95LotOffset(unsigned h,float halfRange) {
    return (m95Rand01(h) * 2.0f - 1.0f) * halfRange;
}
static bool m95RectsOverlap(float ax,float az,float ahx,float ahz,
                            float bx,float bz,float bhx,float bhz,
                            float gap) {
    return std::fabs(ax-bx) < (ahx+bhx+gap) &&
           std::fabs(az-bz) < (ahz+bhz+gap);
}

bool VitaRenderer::init() {
    if (vita2d_init() < 0) return false;
    gM94DebugFont = vita2d_load_default_pgf();

    // M100: load the 512x512 atlas once. Embedded PNG avoids VPK path issues.
    gM100WorldAtlas = vita2d_load_PNG_buffer(kM102WorldAtlasPng);
    if(gM100WorldAtlas) {
        vita2d_texture_set_filters(
            gM100WorldAtlas,
            SCE_GXM_TEXTURE_FILTER_LINEAR,
            SCE_GXM_TEXTURE_FILTER_LINEAR);
    }

    vita2d_set_clear_color(static_cast<unsigned>(RGBA8(110, 175, 225, 255)));
    return true;
}

void VitaRenderer::shutdown() {
    // vita2d_fini() finishes queued GPU work before the texture is released.
    vita2d_fini();
    if(gM100WorldAtlas) {
        vita2d_free_texture(gM100WorldAtlas);
        gM100WorldAtlas=nullptr;
    }
}


// M95 stable city-cell building pass.
// World coordinates are computed from wc.x/wc.z and remain identical while
// walking, flying, rotating the camera, unloading and reloading the cell.
static void m95DrawStableCellBuildings(const Camera& cam,const WorldCell& wc) {
    const float cell = (float)World::CellSizeMeters;
    const float x0 = (float)wc.x * cell;
    const float z0 = (float)wc.z * cell;

    M95PlacedBuilding placed[8];
    int placedCount = 0;

    // Fixed lot centers inside each 64 m cell. Jitter is deterministic and
    // deliberately small, so buildings cannot jump between lots.
    static const float lotX[4] = {12.0f, 26.0f, 42.0f, 54.0f};
    static const float lotZ[4] = {12.0f, 52.0f, 12.0f, 52.0f};

    for(int i=0;i<4;i++) {
        unsigned h=m95HashCell(wc.x,wc.z,0x9500u+(unsigned)i*97u);
        float sx=7.0f + m95Rand01(h^0x12u)*4.0f;
        float sz=7.0f + m95Rand01(h^0x34u)*4.0f;
        float sy=7.0f + m95Rand01(h^0x56u)*18.0f;

        float x=x0+lotX[i]+m95LotOffset(h^0x78u,1.25f);
        float z=z0+lotZ[i]+m95LotOffset(h^0x9au,1.25f);

        // Keep footprints inside their world cell and reject overlap.
        const float margin=3.0f;
        float minX=x0+margin+sx*.5f, maxX=x0+cell-margin-sx*.5f;
        float minZ=z0+margin+sz*.5f, maxZ=z0+cell-margin-sz*.5f;
        if(x<minX)x=minX; if(x>maxX)x=maxX;
        if(z<minZ)z=minZ; if(z>maxZ)z=maxZ;

        if(!m95CanPlace(placed,placedCount,x,z,sx,sz)) continue;

        placed[placedCount++]={x,z,sx,sz};

        unsigned front=RGBA8(158+(h&31),160+((h>>5)&25),166+((h>>10)&24),255);
        unsigned side =RGBA8(125+(h&25),128+((h>>6)&22),135+((h>>11)&20),255);
        unsigned top  =RGBA8(188+(h&20),188+((h>>7)&20),192+((h>>13)&18),255);
        bevelBuilding({x,0.0f,z},sx,sy,sz,cam,front,side,top);
        m92WindowsRoundedFacade({x,0.0f,z},sx,sy,sz,cam);
    }
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

    // M102: the current renderer world is a flat streamed prototype.
    // Capture Dash's real collision-ground spawn height once and anchor the
    // visual roads/buildings/trees/NPCs to that same fixed Y.
    if(!gM102WorldBaseCaptured && !player.inVehicle) {
        gM102WorldBaseY=player.position.y;
        gM102WorldBaseCaptured=true;
    }

    // M90 full-map Vita streaming: pure geometry only.
    // Buildings use separated lots so their footprints never overlap.
    drawTestCity(camera);

    // Tiny center marker.
    vita2d_draw_rectangle(
        477.0f, 269.0f, 6.0f, 6.0f,
        static_cast<unsigned>(RGBA8(255,255,255,255))
    );

    m94DrawCoordinatesHud(player);
    // M97: Dash is visible in third-person and stands on collision ground.
    m97DrawDash(player,camera);
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
