#include "platform/vita/VitaRenderer.h"
#include <vita2d.h>
#include <psp2/gxm.h>
#include <cmath>
#include <cstdio>

namespace {
constexpr float W = 960.0f;
constexpr float H = 544.0f;
constexpr float FOCAL = 620.0f;

struct P2 { float x, y, z; bool ok; };

Vec3 sub(const Vec3& a, const Vec3& b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
float dot3(const Vec3& a,const Vec3& b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
Vec3 cross3(const Vec3& a,const Vec3& b){ return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }

P2 project(const Vec3& p, const Camera& cam) {
    Vec3 f = normalize(sub(cam.target, cam.position));
    Vec3 up{0,1,0};
    Vec3 r = normalize(cross3(f, up));
    Vec3 u = cross3(r, f);
    Vec3 rel = sub(p, cam.position);
    float cx = dot3(rel, r);
    float cy = dot3(rel, u);
    float cz = dot3(rel, f);
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(cz) || cz < 0.20f)
        return {0,0,cz,false};

    float sx = W*0.5f + cx * FOCAL / cz;
    float sy = H*0.52f - cy * FOCAL / cz;

    // Reject pathological projected coordinates before they reach GXM.
    if (!std::isfinite(sx) || !std::isfinite(sy) ||
        sx < -4096.0f || sx > 4096.0f ||
        sy < -4096.0f || sy > 4096.0f)
        return {0,0,cz,false};

    return {sx, sy, cz, true};
}

void tri(const P2& a,const P2& b,const P2& c,unsigned int color) {
    if(!a.ok || !b.ok || !c.ok) return;

    // GPU-safe Vita2D path: do not pass a temporary stack vertex array to GXM.
    // vita2d_draw_triangle() uses Vita2D's managed vertex pool.
    vita2d_draw_triangle(a.x, a.y, b.x, b.y, c.x, c.y, color);
}

void quad(const Vec3& a,const Vec3& b,const Vec3& c,const Vec3& d,const Camera& cam,unsigned int color) {
    P2 pa=project(a,cam), pb=project(b,cam), pc=project(c,cam), pd=project(d,cam);
    tri(pa,pb,pc,color); tri(pa,pc,pd,color);
}

void box(const Vec3& center,float sx,float sy,float sz,const Camera& cam,
         unsigned int front,unsigned int side,unsigned int top) {
    float x0=center.x-sx*.5f,x1=center.x+sx*.5f;
    float y0=center.y,y1=center.y+sy;
    float z0=center.z-sz*.5f,z1=center.z+sz*.5f;
    Vec3 p000{x0,y0,z0}, p100{x1,y0,z0}, p110{x1,y1,z0}, p010{x0,y1,z0};
    Vec3 p001{x0,y0,z1}, p101{x1,y0,z1}, p111{x1,y1,z1}, p011{x0,y1,z1};
    quad(p000,p100,p110,p010,cam,front);
    quad(p101,p001,p011,p111,cam,front);
    quad(p001,p000,p010,p011,cam,side);
    quad(p100,p101,p111,p110,cam,side);
    quad(p010,p110,p111,p011,cam,top);
}

unsigned hash2(int x,int z){ unsigned h=(unsigned)(x*374761393 + z*668265263); h=(h^(h>>13))*1274126177u; return h^(h>>16); }
unsigned shade(unsigned base,int delta){
    int r=(base)&255,g=(base>>8)&255,b=(base>>16)&255,a=(base>>24)&255;
    r=std::max(0,std::min(255,r+delta)); g=std::max(0,std::min(255,g+delta)); b=std::max(0,std::min(255,b+delta));
    return RGBA8(r,g,b,a);
}

void sidewalkBlock(float x0,float z0,float x1,float z1,const Camera& cam) {
    const unsigned concrete=RGBA8(142,140,135,255);
    quad({x0,0.035f,z0},{x1,0.035f,z0},{x1,0.035f,z1},{x0,0.035f,z1},cam,concrete);
}

void roadStripX(float x0,float z0,float x1,float z1,const Camera& cam,bool centerLine) {
    const unsigned asphalt=RGBA8(38,40,43,255);
    quad({x0,0.045f,z0},{x1,0.045f,z0},{x1,0.045f,z1},{x0,0.045f,z1},cam,asphalt);
    if(centerLine){
        float mid=(z0+z1)*0.5f;
        const unsigned line=RGBA8(220,205,105,255);
        for(float x=x0+3.0f;x<x1-1.0f;x+=7.0f)
            quad({x,0.055f,mid-.08f},{x+3.3f,0.055f,mid-.08f},{x+3.3f,0.055f,mid+.08f},{x,0.055f,mid+.08f},cam,line);
    }
}

void roadStripZ(float x0,float z0,float x1,float z1,const Camera& cam,bool centerLine) {
    const unsigned asphalt=RGBA8(38,40,43,255);
    quad({x0,0.046f,z0},{x1,0.046f,z0},{x1,0.046f,z1},{x0,0.046f,z1},cam,asphalt);
    if(centerLine){
        float mid=(x0+x1)*0.5f;
        const unsigned line=RGBA8(220,205,105,255);
        for(float z=z0+3.0f;z<z1-1.0f;z+=7.0f)
            quad({mid-.08f,0.056f,z},{mid+.08f,0.056f,z},{mid+.08f,0.056f,z+3.3f},{mid-.08f,0.056f,z+3.3f},cam,line);
    }
}

void facadeWindows(const Vec3& c,float sx,float sy,float sz,const Camera& cam,unsigned seed,bool downtown) {
    if(sy < 7.0f) return;
    int floors=(int)(sy/3.0f);
    if(floors>12) floors=12; // Vita draw-call budget.
    int cols= downtown ? 3 : 2;
    const unsigned glassA=RGBA8(80,116,137,255);
    const unsigned glassB=RGBA8(168,178,157,255);
    float x0=c.x-sx*.5f, z0=c.z-sz*.5f;
    for(int f=1;f<floors;f++) {
        float y=1.0f+f*(sy/(float)floors);
        for(int k=0;k<cols;k++) {
            unsigned q=seed ^ (unsigned)(f*131+k*911);
            unsigned col=(q&1)?glassA:glassB;
            float fx=x0+(k+1)*(sx/(cols+1));
            float fz=z0+(k+1)*(sz/(cols+1));
            quad({fx-.55f,y,z0-.012f},{fx+.55f,y,z0-.012f},{fx+.55f,y+.85f,z0-.012f},{fx-.55f,y+.85f,z0-.012f},cam,col);
            quad({x0-.012f,y,fz-.55f},{x0-.012f,y,fz+.55f},{x0-.012f,y+.85f,fz+.55f},{x0-.012f,y+.85f,fz-.55f},cam,shade(col,-18));
        }
    }
}

void building(const Vec3& c,float sx,float sy,float sz,const Camera& cam,unsigned seed,RegionType region) {
    static const unsigned palette[] = {
        static_cast<unsigned>(RGBA8(124,126,128,255)),
        static_cast<unsigned>(RGBA8(148,139,127,255)),
        static_cast<unsigned>(RGBA8(112,123,133,255)),
        static_cast<unsigned>(RGBA8(153,151,143,255)),
        static_cast<unsigned>(RGBA8(126,113,105,255)),
        static_cast<unsigned>(RGBA8(138,145,150,255))
    };
    unsigned face=palette[seed%6];
    bool downtown=region==RegionType::Downtown;
    box(c,sx,sy,sz,cam,face,shade(face,-22),shade(face,18));
    facadeWindows(c,sx,sy,sz,cam,seed,downtown);

    // Roof parapet / HVAC silhouette to make the skyline read better.
    if(sy>14.0f){
        box({c.x,c.y+sy,c.z},sx*.86f,.42f,sz*.86f,cam,shade(face,9),shade(face,-10),shade(face,25));
        if((seed&3)==0)
            box({c.x+sx*.18f,c.y+sy+.42f,c.z-sz*.14f},std::max(1.2f,sx*.18f),.75f,std::max(1.2f,sz*.16f),cam,RGBA8(96,99,98,255),RGBA8(72,75,75,255),RGBA8(129,132,130,255));
    }

    // Street-level base gives buildings a stronger grounded look.
    box({c.x,c.y,c.z},sx*1.03f,.55f,sz*1.03f,cam,shade(face,-28),shade(face,-36),shade(face,-12));
}



enum class StoreType { GunStore, GeneralStore };

void storefront(const Vec3& c,float sx,float sy,float sz,const Camera& cam,unsigned seed,StoreType type) {
    unsigned wall = type==StoreType::GunStore ? RGBA8(82,86,82,255) : RGBA8(174,145,96,255);
    unsigned trim = type==StoreType::GunStore ? RGBA8(40,43,40,255) : RGBA8(112,75,44,255);
    unsigned glass = RGBA8(76,115,132,255);
    unsigned sign = type==StoreType::GunStore ? RGBA8(145,46,38,255) : RGBA8(45,126,78,255);
    box(c,sx,sy,sz,cam,wall,shade(wall,-20),shade(wall,15));
    // Dark foundation and canopy make both shops readable at Vita resolution.
    box({c.x,c.y,c.z},sx*1.04f,.45f,sz*1.04f,cam,trim,shade(trim,-10),shade(trim,8));
    float frontZ=c.z-sz*.5f-.015f;
    quad({c.x-sx*.40f,.65f,frontZ},{c.x-sx*.06f,.65f,frontZ},{c.x-sx*.06f,2.55f,frontZ},{c.x-sx*.40f,2.55f,frontZ},cam,glass);
    quad({c.x+sx*.06f,.65f,frontZ},{c.x+sx*.40f,.65f,frontZ},{c.x+sx*.40f,2.55f,frontZ},{c.x+sx*.06f,2.55f,frontZ},cam,glass);
    quad({c.x-sx*.42f,2.85f,frontZ-.02f},{c.x+sx*.42f,2.85f,frontZ-.02f},{c.x+sx*.42f,3.55f,frontZ-.02f},{c.x-sx*.42f,3.55f,frontZ-.02f},cam,sign);
    // Door marker / frame.
    box({c.x,0,c.z-sz*.5f-.04f},1.25f,2.45f,.12f,cam,shade(glass,-24),trim,shade(glass,8));
    if(type==StoreType::GunStore) {
        // Neutral shield-like storefront marker; inventory/interior comes in a later gameplay milestone.
        box({c.x,3.05f,c.z-sz*.5f-.09f},.85f,.34f,.08f,cam,RGBA8(215,215,202,255),RGBA8(180,180,170,255),RGBA8(235,235,225,255));
    } else {
        // General-store awning strips.
        for(int i=-2;i<=2;i++) box({c.x+i*1.15f,2.58f,c.z-sz*.5f-.20f},.72f,.16f,.42f,cam,(i&1)?sign:RGBA8(224,215,184,255),trim,shade(sign,10));
    }
    (void)seed;
}

void drawStreamingCity(const Camera& cam,const World& world) {
    const float cell=(float)World::CellSizeMeters;
    const float lot=16.0f;
    for(const WorldCell& wc: world.cells) {
        float x0=wc.x*cell, z0=wc.z*cell;
        unsigned int ground=RGBA8(76,101,70,255);
        if(wc.region==RegionType::Downtown || wc.region==RegionType::Urban) ground=RGBA8(72,73,73,255);
        else if(wc.region==RegionType::Suburb) ground=RGBA8(94,110,82,255);
        else if(wc.region==RegionType::Industrial) ground=RGBA8(78,78,73,255);
        else if(wc.region==RegionType::Airport) ground=RGBA8(89,91,91,255);
        else if(wc.region==RegionType::Coast) ground=RGBA8(174,158,112,255);
        quad({x0,0,z0},{x0+cell,0,z0},{x0+cell,0,z0+cell},{x0,0,z0+cell},cam,ground);

        if(wc.region==RegionType::Downtown || wc.region==RegionType::Urban || wc.region==RegionType::Suburb) {
            bool arterialX=(wc.x%4==0), arterialZ=(wc.z%4==0);

            // Continuous sidewalk pads first, then roads cut visually over them.
            if(wc.region!=RegionType::Suburb)
                sidewalkBlock(x0+1.0f,z0+1.0f,x0+cell-1.0f,z0+cell-1.0f,cam);

            if(arterialX) roadStripZ(x0+24.5f,z0,x0+39.5f,z0+cell,cam,true);
            if(arterialZ) roadStripX(x0,z0+24.5f,x0+cell,z0+39.5f,cam,true);

            unsigned h=wc.seed;
            for(int iz=0;iz<4;iz++) for(int ix=0;ix<4;ix++) {
                float bx=x0+8+ix*lot, bz=z0+8+iz*lot;
                if((arterialX && bx>x0+22 && bx<x0+42)||(arterialZ && bz>z0+22 && bz<z0+42)) continue;
                unsigned q=hash2(wc.x*7+ix,wc.z*7+iz)^h;
                // Deterministic neighborhood shops. They stream exactly like other buildings.
                // General stores are more common; gun stores are intentionally sparse.
                if(wc.region!=RegionType::Suburb && (q % 113u)==7u) {
                    storefront({bx,0,bz},11.0f,5.2f,10.0f,cam,q,StoreType::GunStore);
                    continue;
                }
                if((q % 47u)==11u) {
                    storefront({bx,0,bz},11.5f,4.8f,10.5f,cam,q,StoreType::GeneralStore);
                    continue;
                }
                if((q&7)==0) {
                    // Pocket park/plaza: simple grass inset or plaza pad.
                    unsigned park=((q>>4)&1)?RGBA8(75,112,67,255):RGBA8(157,151,137,255);
                    quad({bx-5,0.065f,bz-5},{bx+5,0.065f,bz-5},{bx+5,0.065f,bz+5},{bx-5,0.065f,bz+5},cam,park);
                    continue;
                }
                float height = wc.region==RegionType::Downtown ? 18.0f+(q%58) :
                               wc.region==RegionType::Urban ? 8.0f+(q%24) : 4.0f+(q%8);
                float sx=8.0f+((q>>8)%5), sz=8.0f+((q>>13)%5);
                building({bx,0,bz},sx,height,sz,cam,q,wc.region);
            }
        }
    }
}

void person(const Player& p,const Camera& cam) {
    box(p.position,0.65f,1.55f,0.50f,cam,RGBA8(40,105,190,255),RGBA8(28,76,145,255),RGBA8(75,135,220,255));
    box({p.position.x,p.position.y+1.55f,p.position.z},0.48f,0.45f,0.48f,cam,RGBA8(202,159,120,255),RGBA8(165,124,91,255),RGBA8(224,183,141,255));
}

void drawVehicleLighting(const Vec3& c,const VehicleLighting& l,const Camera& cam,float heading) {
    const float sx=std::sin(heading), cz=std::cos(heading);
    Vec3 front={c.x+sx*1.78f,c.y+.35f,c.z+cz*1.78f};
    Vec3 rear={c.x-sx*1.78f,c.y+.32f,c.z-cz*1.78f};
    if(l.lights!=VehicleLightMode::Off){
        unsigned col=l.lights==VehicleLightMode::HighBeam?RGBA8(255,248,205,255):RGBA8(236,224,180,255);
        box({front.x+.55f*cz,front.y,front.z-.55f*sx},.18f,.18f,.18f,cam,col,col,col);
        box({front.x-.55f*cz,front.y,front.z+.55f*sx},.18f,.18f,.18f,cam,col,col,col);
    }
    if(l.blinkVisible && l.indicators!=IndicatorMode::Off){
        unsigned amber=RGBA8(255,157,38,255);
        bool left=l.indicators==IndicatorMode::Left||l.indicators==IndicatorMode::Hazards;
        bool right=l.indicators==IndicatorMode::Right||l.indicators==IndicatorMode::Hazards;
        if(left) box({front.x-.70f*cz,front.y+.03f,front.z+.70f*sx},.18f,.18f,.18f,cam,amber,amber,amber);
        if(right) box({front.x+.70f*cz,front.y+.03f,front.z-.70f*sx},.18f,.18f,.18f,cam,amber,amber,amber);
        if(left) box({rear.x+.70f*cz,rear.y,rear.z-.70f*sx},.18f,.18f,.18f,cam,amber,amber,amber);
        if(right) box({rear.x-.70f*cz,rear.y,rear.z+.70f*sx},.18f,.18f,.18f,cam,amber,amber,amber);
    }
}

void vehicle(const Vehicle& v,const Camera& cam) {
    Vec3 c=v.position;
    box(c,1.75f,0.65f,3.4f,cam,RGBA8(180,44,38,255),RGBA8(135,30,28,255),RGBA8(216,64,55,255));
    box({c.x,c.y+0.62f,c.z},1.45f,0.52f,1.75f,cam,RGBA8(56,73,85,255),RGBA8(42,55,65,255),RGBA8(80,101,115,255));
    drawVehicleLighting(c,v.lighting,cam,v.heading);
}

void trafficVehicle(const TrafficCar& v,const Camera& cam) {
    unsigned body=RGBA8(132,139,145,255);
    if(v.type==TrafficVehicleType::Taxi) body=RGBA8(214,185,64,255);
    else if(v.type==TrafficVehicleType::Bus) body=RGBA8(55,115,162,255);
    else if(v.type==TrafficVehicleType::Truck) body=RGBA8(126,108,83,255);
    else if(v.type==TrafficVehicleType::Motorcycle) body=RGBA8(52,52,56,255);
    float w=v.type==TrafficVehicleType::Motorcycle?.75f:(v.type==TrafficVehicleType::Bus||v.type==TrafficVehicleType::Truck?2.2f:1.7f);
    float len=v.type==TrafficVehicleType::Bus?6.2f:(v.type==TrafficVehicleType::Truck?5.4f:(v.type==TrafficVehicleType::Motorcycle?1.7f:3.4f));
    box(v.position,w,.62f,len,cam,body,shade(body,-22),shade(body,18));
    drawVehicleLighting(v.position,v.lighting,cam,v.heading);
}

void pedestrian(const NPC& n,const Camera& cam){
    unsigned body=n.state==NPCState::Fleeing?RGBA8(184,72,67,255):RGBA8(69,107,155,255);
    box(n.position,.52f,1.45f,.42f,cam,body,shade(body,-18),shade(body,15));
    box({n.position.x,n.position.y+1.45f,n.position.z},.40f,.40f,.40f,cam,RGBA8(202,159,120,255),RGBA8(165,124,91,255),RGBA8(224,183,141,255));
}
}

bool VitaRenderer::init() {
    if (vita2d_init() < 0) return false;
    vita2d_set_clear_color(RGBA8(118,183,224,255));
    return true;
}

void VitaRenderer::shutdown() {
    // Finish queued GPU work before releasing Vita2D/GXM resources.
    sceGxmFinish();
    vita2d_fini();
}

void VitaRenderer::draw(const Player& player,const Vehicle& car,const Camera& camera,
                        const World& world,const WantedSystem& wanted,
                        const TrafficSystem& traffic,const NPCSystem& npcs,float fps) {
    vita2d_start_drawing();
    vita2d_clear_screen();

    // M63 physical-Vita stability gate:
    // render the streamed world only after the safe primitive path is active.
    drawStreamingCity(camera, world);
    for(const auto& t: traffic.cars) trafficVehicle(t,camera);
    for(const auto& n: npcs.npcs) pedestrian(n,camera);
    vehicle(car,camera);
    if(!player.inVehicle && camera.mode!=CameraMode::FirstPerson) person(player,camera);

    vita2d_draw_rectangle(18,18,300,62,RGBA8(0,0,0,165));
    vita2d_draw_rectangle(29,58,210,8,RGBA8(30,30,30,220));
    vita2d_draw_rectangle(29,58,210*(1.0f-wanted.heat/100.0f),8,RGBA8(88,210,112,255));
    for(int i=0;i<wanted.level;i++) vita2d_draw_fill_circle(264+i*10,39,4,RGBA8(255,210,60,255));
    vita2d_draw_line(476,272,484,272,RGBA8(255,255,255,160));
    vita2d_draw_line(480,268,480,276,RGBA8(255,255,255,160));

    (void)fps;
    vita2d_end_drawing();
    vita2d_swap_buffers();
}
