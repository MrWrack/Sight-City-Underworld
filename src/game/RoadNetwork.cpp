#include "game/RoadNetwork.h"
#include <cmath>
#include <queue>
#include <limits>
#include <algorithm>

static float d2(const Vec3&a,const Vec3&b){float x=a.x-b.x,z=a.z-b.z;return x*x+z*z;}

void RoadNetwork::buildAround(const Vec3& c,RegionType r){
    center_=c; region_=r; nodes_.clear();
    float spacing=(r==RegionType::Downtown||r==RegionType::Urban)?64.0f:(r==RegionType::Suburb?96.0f:128.0f);
    int radius=(r==RegionType::Downtown||r==RegionType::Urban)?5:4;
    int width=radius*2+1;
    nodes_.reserve(width*width);
    for(int z=-radius;z<=radius;z++) for(int x=-radius;x<=radius;x++){
        RoadNode n; n.position={c.x+x*spacing,0,c.z+z*spacing};
        n.speedLimit=(r==RegionType::Downtown)?11.0f:(r==RegionType::Urban?13.0f:(r==RegionType::Suburb?15.0f:18.0f));
        n.intersection=true; nodes_.push_back(n);
    }
    auto idx=[&](int x,int z){return (z+radius)*width+(x+radius);};
    for(int z=-radius;z<=radius;z++) for(int x=-radius;x<=radius;x++){
        auto& n=nodes_[idx(x,z)];
        if(x>-radius)n.links.push_back(idx(x-1,z));
        if(x< radius)n.links.push_back(idx(x+1,z));
        if(z>-radius)n.links.push_back(idx(x,z-1));
        if(z< radius)n.links.push_back(idx(x,z+1));
    }
}
int RoadNetwork::nearestNode(const Vec3&p) const{int best=-1;float bd=std::numeric_limits<float>::max();for(int i=0;i<(int)nodes_.size();i++){float v=d2(nodes_[i].position,p);if(v<bd){bd=v;best=i;}}return best;}
const RoadNode* RoadNetwork::node(int i) const{return i>=0&&i<(int)nodes_.size()?&nodes_[i]:nullptr;}
RoadRoute RoadNetwork::route(const Vec3& from,const Vec3& to) const{
    RoadRoute out; int s=nearestNode(from),g=nearestNode(to); if(s<0||g<0)return out;
    std::vector<int> prev(nodes_.size(),-1); std::queue<int> q; q.push(s); prev[s]=s;
    while(!q.empty()){int u=q.front();q.pop();if(u==g)break;for(int v:nodes_[u].links)if(prev[v]<0){prev[v]=u;q.push(v);}}
    if(prev[g]<0)return out; for(int at=g;;at=prev[at]){out.nodes.push_back(at);if(at==s)break;} std::reverse(out.nodes.begin(),out.nodes.end()); return out;
}
