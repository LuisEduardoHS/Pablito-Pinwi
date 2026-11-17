#ifndef GC_GAMEPLAY_INTERACTION_H
#define GC_GAMEPLAY_INTERACTION_H
#include <cmath>
#include <string>
#include <vector>

namespace Alchemy
{
struct Position
{
    float x,y,z;
    Position(float x=0,float y=0,float z=0):x(x),y(y),z(z){}
};
enum class TargetKind { Item, Mortar, Cauldron, Hearth, Bottler, Delivery, Cloud };
struct Target
{
    TargetKind kind;
    int index,id;
    Position position;
    float range;
    std::wstring label;
    bool visible;
    Target(TargetKind kind=TargetKind::Item,int index=-1,int id=-1,Position position=Position(),
        std::wstring label=L"",float range=3.5f):kind(kind),index(index),id(id),position(position),range(range),label(label),visible(true){}
};

// Shared by items, stations, deliveries and the existing cloud. One winner per E.
inline int SelectTarget(const Position& player,const Position& forward,const std::vector<Target>& targets)
{
    int best=-1;float bestScore=1e9f;
    const float forwardLength=sqrtf(forward.x*forward.x+forward.z*forward.z);
    for(size_t i=0;i<targets.size();++i)
    {
        const Target& target=targets[i];
        const float dx=target.position.x-player.x,dz=target.position.z-player.z,dy=target.position.y-player.y;
        const float distance=sqrtf(dx*dx+dz*dz);
        if(!target.visible || distance>target.range || fabsf(dy)>3.8f) continue;
        const float facing=distance>0.001f && forwardLength>0.001f?(dx*forward.x+dz*forward.z)/(distance*forwardLength):1;
        const float score=distance+(1-facing)*target.range*0.25f+fabsf(dy)*0.08f;
        if(score<bestScore-0.001f || (fabsf(score-bestScore)<0.001f && (best<0 || target.id<targets[best].id)))
        {best=static_cast<int>(i);bestScore=score;}
    }
    return best;
}
}
#endif
