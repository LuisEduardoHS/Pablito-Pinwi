#ifndef GC_SCENE_COLLISION_H
#define GC_SCENE_COLLISION_H

#include <d3dx10.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <vector>

namespace SceneCollision
{
inline float Dot(const D3DXVECTOR3& a, const D3DXVECTOR3& b) { return D3DXVec3Dot(&a, &b); }
inline D3DXVECTOR3 Cross(const D3DXVECTOR3& a, const D3DXVECTOR3& b)
{ D3DXVECTOR3 n; D3DXVec3Cross(&n, &a, &b); return n; }
inline float Length(const D3DXVECTOR3& a) { return sqrtf(Dot(a,a)); }
inline float Clamp(float t, float lo = 0, float hi = 1) { return fmaxf(lo,fminf(hi,t)); }
inline D3DXVECTOR3 Transform(const D3DXVECTOR3& v, const D3DXMATRIX& m)
{ D3DXVECTOR3 p; D3DXVec3TransformCoord(&p,&v,&m); return p; }

struct Bounds
{
    D3DXVECTOR3 low, high;
    Bounds() : low(FLT_MAX,FLT_MAX,FLT_MAX), high(-FLT_MAX,-FLT_MAX,-FLT_MAX) {}
    void Add(const D3DXVECTOR3& p)
    {
        low.x=fminf(low.x,p.x); low.y=fminf(low.y,p.y); low.z=fminf(low.z,p.z);
        high.x=fmaxf(high.x,p.x); high.y=fmaxf(high.y,p.y); high.z=fmaxf(high.z,p.z);
    }
    void Expand(float r) { low-=D3DXVECTOR3(r,r,r); high+=D3DXVECTOR3(r,r,r); }
    bool Overlaps(const Bounds& b) const
    { return low.x<=b.high.x && high.x>=b.low.x && low.y<=b.high.y && high.y>=b.low.y && low.z<=b.high.z && high.z>=b.low.z; }
    Bounds Transformed(const D3DXMATRIX& m) const
    {
        Bounds b;
        for(int i=0;i<8;++i) b.Add(Transform(D3DXVECTOR3(i&1?high.x:low.x,i&2?high.y:low.y,i&4?high.z:low.z),m));
        return b;
    }
};

struct Triangle
{
    D3DXVECTOR3 a,b,c;
    Triangle(const D3DXVECTOR3& a,const D3DXVECTOR3& b,const D3DXVECTOR3& c) : a(a),b(b),c(c) {}
    Bounds GetBounds() const { Bounds result; result.Add(a);result.Add(b);result.Add(c);return result; }
};

// Two-sided intersection: imported meshes need not share a winding convention.
inline bool RayTriangle(const D3DXVECTOR3& start,const D3DXVECTOR3& delta,const Triangle& tri,float& t)
{
    const D3DXVECTOR3 ab=tri.b-tri.a, ac=tri.c-tri.a;
    const D3DXVECTOR3 p=Cross(delta,ac);
    const float determinant=Dot(ab,p);
    if(fabsf(determinant)<1e-8f) return false;
    const float inverse=1/determinant;
    const D3DXVECTOR3 offset=start-tri.a;
    const float u=Dot(offset,p)*inverse;
    if(u<0 || u>1) return false;
    const D3DXVECTOR3 q=Cross(offset,ab);
    const float v=Dot(delta,q)*inverse;
    if(v<0 || u+v>1) return false;
    t=Dot(ac,q)*inverse;
    return t>=0 && t<=1;
}

inline D3DXVECTOR3 ClosestPoint(const D3DXVECTOR3& p,const Triangle& tri)
{
    const D3DXVECTOR3 ab=tri.b-tri.a, ac=tri.c-tri.a, ap=p-tri.a;
    float d1=Dot(ab,ap), d2=Dot(ac,ap);
    if(d1<=0 && d2<=0) return tri.a;
    const D3DXVECTOR3 bp=p-tri.b;
    float d3=Dot(ab,bp), d4=Dot(ac,bp);
    if(d3>=0 && d4<=d3) return tri.b;
    float vc=d1*d4-d3*d2;
    if(vc<=0 && d1>=0 && d3<=0) return tri.a+ab*(d1/(d1-d3));
    const D3DXVECTOR3 cp=p-tri.c;
    float d5=Dot(ab,cp), d6=Dot(ac,cp);
    if(d6>=0 && d5<=d6) return tri.c;
    float vb=d5*d2-d1*d6;
    if(vb<=0 && d2>=0 && d6<=0) return tri.a+ac*(d2/(d2-d6));
    float va=d3*d6-d5*d4;
    if(va<=0 && d4-d3>=0 && d5-d6>=0) return tri.b+(tri.c-tri.b)*((d4-d3)/((d4-d3)+(d5-d6)));
    float inverse=1/(va+vb+vc);
    return tri.a+ab*(vb*inverse)+ac*(vc*inverse);
}

inline void ClosestSegments(const D3DXVECTOR3& a,const D3DXVECTOR3& b,
    const D3DXVECTOR3& c,const D3DXVECTOR3& d,D3DXVECTOR3& p,D3DXVECTOR3& q)
{
    const D3DXVECTOR3 u=b-a,v=d-c,r=a-c;
    float uu=Dot(u,u),vv=Dot(v,v),uv=Dot(u,v),ur=Dot(u,r),vr=Dot(v,r),s=0,t=0;
    if(uu<1e-10f && vv<1e-10f) {p=a;q=c;return;}
    if(uu<1e-10f) t=Clamp(vr/vv);
    else if(vv<1e-10f) s=Clamp(-ur/uu);
    else
    {
        float denominator=uu*vv-uv*uv;
        if(denominator>1e-10f) s=Clamp((uv*vr-ur*vv)/denominator);
        t=(uv*s+vr)/vv;
        if(t<0) { t=0;s=Clamp(-ur/uu); }
        else if(t>1) { t=1;s=Clamp((uv-ur)/uu); }
    }
    p=a+u*s; q=c+v*t;
}

inline float SegmentTriangle(const D3DXVECTOR3& a,const D3DXVECTOR3& b,const Triangle& tri,
    D3DXVECTOR3& onSegment,D3DXVECTOR3& onTriangle)
{
    float t;
    if(RayTriangle(a,b-a,tri,t)) {onSegment=onTriangle=a+(b-a)*t;return 0;}
    onSegment=a; onTriangle=ClosestPoint(a,tri);
    float best=Dot(a-onTriangle,a-onTriangle);
    const D3DXVECTOR3 end=ClosestPoint(b,tri);
    float distance=Dot(b-end,b-end);
    if(distance<best) {best=distance;onSegment=b;onTriangle=end;}
    const D3DXVECTOR3 points[]={tri.a,tri.b,tri.c};
    for(int i=0;i<3;++i)
    {
        D3DXVECTOR3 p,q;
        ClosestSegments(a,b,points[i],points[(i+1)%3],p,q);
        distance=Dot(p-q,p-q);
        if(distance<best) {best=distance;onSegment=p;onTriangle=q;}
    }
    return sqrtf(fmaxf(0,best));
}

class MeshCollider
{
    struct Node { Bounds bounds; unsigned begin,count; int left,right; };
    std::vector<Triangle> triangles;
    std::vector<Node> nodes;
    D3DXMATRIX world, inverse;
    Bounds worldBounds;
    bool enabled=true, validTransform=false, registered=false;

    static std::vector<MeshCollider*>& Registry() { static std::vector<MeshCollider*> all; return all; }
    int BuildNode(unsigned begin,unsigned count)
    {
        const int index=static_cast<int>(nodes.size());
        Node node; node.begin=begin;node.count=count;node.left=node.right=-1;
        for(unsigned i=begin;i<begin+count;++i) {node.bounds.Add(triangles[i].GetBounds().low);node.bounds.Add(triangles[i].GetBounds().high);}
        nodes.push_back(node);
        if(count>8)
        {
            const D3DXVECTOR3 extent=node.bounds.high-node.bounds.low;
            const int axis=extent.x>extent.y?(extent.x>extent.z?0:2):(extent.y>extent.z?1:2);
            const unsigned middle=begin+count/2;
            std::nth_element(triangles.begin()+begin,triangles.begin()+middle,triangles.begin()+begin+count,
                [axis](const Triangle& a,const Triangle& b) {
                    const D3DXVECTOR3 ca=a.a+a.b+a.c, cb=b.a+b.b+b.c;
                    return ca[axis]<cb[axis];
                });
            const int left=BuildNode(begin,middle-begin),right=BuildNode(middle,begin+count-middle);
            nodes[index].left=left;nodes[index].right=right;
        }
        return index;
    }
    template<typename Visitor> void Visit(int index,const Bounds& localBounds,Visitor& visitor) const
    {
        const Node& node=nodes[index];
        if(!node.bounds.Overlaps(localBounds)) return;
        if(node.left>=0) {Visit(node.left,localBounds,visitor);Visit(node.right,localBounds,visitor);return;}
        for(unsigned i=node.begin;i<node.begin+node.count;++i)
        {
            const Triangle& t=triangles[i];
            visitor(Triangle(Transform(t.a,world),Transform(t.b,world),Transform(t.c,world)));
        }
    }
public:
    MeshCollider() {D3DXMatrixIdentity(&world);D3DXMatrixIdentity(&inverse);}
    MeshCollider(const MeshCollider&)=delete;
    MeshCollider& operator=(const MeshCollider&)=delete;
    ~MeshCollider() {Clear();}
    void Clear()
    {
        if(registered) {auto& all=Registry();all.erase(std::remove(all.begin(),all.end(),this),all.end());registered=false;}
        triangles.clear();nodes.clear();validTransform=false;
    }
    void Build(const std::vector<Triangle>& source)
    {
        Clear();
        for(const Triangle& t:source)
        {
            const D3DXVECTOR3 n=Cross(t.b-t.a,t.c-t.a);
            if(std::isfinite(Dot(n,n)) && Dot(n,n)>1e-12f) triangles.push_back(t);
        }
        if(triangles.empty()) return;
        BuildNode(0,static_cast<unsigned>(triangles.size()));
        Registry().push_back(this);registered=true;
        const D3DXMATRIX initial=world;validTransform=false;SetWorld(initial);
    }
    void SetEnabled(bool value) {enabled=value;}
    bool IsEnabled() const {return enabled && validTransform && !nodes.empty();}
    size_t TriangleCount() const {return triangles.size();}
    const Bounds& GetWorldBounds() const {return worldBounds;}
    static const std::vector<MeshCollider*>& All() {return Registry();}
    void SetWorld(const D3DXMATRIX& matrix)
    {
        if(validTransform && std::memcmp(&world,&matrix,sizeof(matrix))==0) return;
        world=matrix;
        float determinant=0;
        validTransform=D3DXMatrixInverse(&inverse,&determinant,&world)!=NULL && std::isfinite(determinant) && fabsf(determinant)>1e-10f;
        if(validTransform && !nodes.empty()) worldBounds=nodes[0].bounds.Transformed(world);
    }
    template<typename Visitor> void Query(const Bounds& bounds,Visitor visitor) const
    {
        if(!IsEnabled() || !bounds.Overlaps(worldBounds)) return;
        Visit(0,bounds.Transformed(inverse),visitor);
    }
};

template<typename Visitor> inline void Query(const Bounds& bounds,Visitor visitor,
    const MeshCollider* ignore=NULL,const MeshCollider* ignoreAlso=NULL)
{
    for(const MeshCollider* model:MeshCollider::All())
        if(model!=ignore && model!=ignoreAlso) model->Query(bounds,visitor);
}

inline bool Raycast(const D3DXVECTOR3& start,const D3DXVECTOR3& end,float& fraction,
    const MeshCollider* ignore=NULL,const MeshCollider* ignoreAlso=NULL,bool walkableOnly=false)
{
    Bounds bounds;bounds.Add(start);bounds.Add(end);bounds.Expand(0.001f);
    fraction=1;bool found=false;
    Query(bounds,[&](const Triangle& tri) {
        D3DXVECTOR3 n=Cross(tri.b-tri.a,tri.c-tri.a);
        if(walkableOnly && fabsf(n.y)<Length(n)*0.65f) return;
        float hit;
        if(RayTriangle(start,end-start,tri,hit) && hit<fraction)
        {fraction=hit;found=true;}
    },ignore,ignoreAlso);
    return found;
}

struct Contacts {bool ground=false,ceiling=false,blocked=false;};

inline Bounds CapsuleBounds(const D3DXVECTOR3& feet,float height,float radius)
{
    Bounds bounds;bounds.Add(feet+D3DXVECTOR3(-radius,0,-radius));
    bounds.Add(feet+D3DXVECTOR3(radius,height,radius));return bounds;
}

inline bool CapsuleContact(const D3DXVECTOR3& feet,float height,float radius,const Triangle& tri,
    const D3DXVECTOR3& previous,D3DXVECTOR3& normal,float& depth)
{
    const D3DXVECTOR3 a=feet+D3DXVECTOR3(0,radius,0),b=feet+D3DXVECTOR3(0,fmaxf(radius,height-radius),0);
    D3DXVECTOR3 onAxis,onTriangle;
    float distance=SegmentTriangle(a,b,tri,onAxis,onTriangle);
    depth=radius-distance;
    if(depth<=0.0001f) return false;
    if(distance>1e-6f) normal=(onAxis-onTriangle)/distance;
    else
    {
        normal=Cross(tri.b-tri.a,tri.c-tri.a);normal/=Length(normal);
        if(Dot(previous+D3DXVECTOR3(0,height*0.5f,0)-tri.a,normal)<0) normal=-normal;
    }
    return true;
}

inline Contacts ResolveCapsule(D3DXVECTOR3& feet,float height,float radius,const D3DXVECTOR3& previous,
    const MeshCollider* ignore=NULL)
{
    Contacts contacts;
    for(int iteration=0;iteration<12;++iteration)
    {
        float deepest=0;D3DXVECTOR3 push(0,0,0);
        Bounds bounds=CapsuleBounds(feet,height,radius);bounds.Expand(0.003f);
        Query(bounds,[&](const Triangle& tri) {
            D3DXVECTOR3 normal;float depth;
            if(CapsuleContact(feet,height,radius,tri,previous,normal,depth) && depth>deepest)
            {deepest=depth;push=normal;}
        },ignore);
        if(deepest<=0) break;
        feet+=push*(deepest+0.002f);
        contacts.ground=contacts.ground || push.y>0.55f;
        contacts.ceiling=contacts.ceiling || push.y< -0.55f;
        contacts.blocked=true;
    }
    return contacts;
}

inline bool CapsuleFree(const D3DXVECTOR3& feet,float height,float radius,const MeshCollider* ignore=NULL)
{
    bool free=true;const Bounds bounds=CapsuleBounds(feet,height,radius);
    Query(bounds,[&](const Triangle& tri) {
        D3DXVECTOR3 n;float depth;
        if(CapsuleContact(feet,height,radius,tri,feet,n,depth) && depth>0.015f) free=false;
    },ignore);
    return free;
}

// Small steps prevent tunneling through a thin triangle at running/vehicle speeds.
inline D3DXVECTOR3 MoveCapsule(D3DXVECTOR3 feet,const D3DXVECTOR3& movement,float height,float radius,
    Contacts& contacts,const MeshCollider* ignore=NULL)
{
    const int steps=(std::max)(1,static_cast<int>(ceilf(Length(movement)/fmaxf(0.05f,radius*0.35f))));
    const D3DXVECTOR3 delta=movement/static_cast<float>(steps);
    for(int i=0;i<steps;++i)
    {
        const D3DXVECTOR3 previous=feet;feet+=delta;
        Contacts hit=ResolveCapsule(feet,height,radius,previous,ignore);
        contacts.ground=contacts.ground||hit.ground;contacts.ceiling=contacts.ceiling||hit.ceiling;contacts.blocked=contacts.blocked||hit.blocked;
    }
    return feet;
}

inline D3DXVECTOR3 RotateY(const D3DXVECTOR3& v,float angle)
{float c=cosf(angle),s=sinf(angle);return D3DXVECTOR3(v.x*c+v.z*s,v.y,-v.x*s+v.z*c);}

// SAT between a triangle and an oriented box, retaining the minimum separation.
inline bool BoxContact(const D3DXVECTOR3& center,const D3DXVECTOR3& half,float yaw,const Triangle& triangle,
    const D3DXVECTOR3& previous,D3DXVECTOR3& normal,float& depth)
{
    const Triangle tri(RotateY(triangle.a-center,-yaw),RotateY(triangle.b-center,-yaw),RotateY(triangle.c-center,-yaw));
    const D3DXVECTOR3 edges[]={tri.b-tri.a,tri.c-tri.b,tri.a-tri.c};
    const D3DXVECTOR3 axes[]={D3DXVECTOR3(1,0,0),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,0,1)};
    D3DXVECTOR3 tests[13]={axes[0],axes[1],axes[2],Cross(edges[0],edges[1])};
    int k=4;for(const auto& edge:edges) for(const auto& axis:axes) tests[k++]=Cross(edge,axis);
    depth=FLT_MAX;
    const D3DXVECTOR3 from=RotateY(previous-center,-yaw);
    for(D3DXVECTOR3 axis:tests)
    {
        const float length=Length(axis);if(length<1e-6f) continue;axis/=length;
        const float a=Dot(tri.a,axis),b=Dot(tri.b,axis),c=Dot(tri.c,axis);
        const float low=fminf(a,fminf(b,c)),high=fmaxf(a,fmaxf(b,c));
        const float radius=fabsf(axis.x)*half.x+fabsf(axis.y)*half.y+fabsf(axis.z)*half.z;
        if(low>=radius || high<= -radius) return false;
        const float positive=high+radius,negative=radius-low;
        const bool plus=fabsf(positive-negative)<1e-5f?Dot(from,axis)>=0:positive<negative;
        const float penetration=plus?positive:negative;
        if(penetration<depth) {depth=penetration;normal=RotateY(plus?axis:-axis,yaw);}
    }
    return depth>0.0001f && depth<FLT_MAX;
}

inline Contacts ResolveBox(D3DXVECTOR3& center,const D3DXVECTOR3& half,float yaw,const D3DXVECTOR3& previous,
    const MeshCollider* ignore=NULL)
{
    Contacts contacts;
    for(int iteration=0;iteration<12;++iteration)
    {
        Bounds bounds;for(int i=0;i<8;++i)
            bounds.Add(center+RotateY(D3DXVECTOR3(i&1?half.x:-half.x,i&2?half.y:-half.y,i&4?half.z:-half.z),yaw));
        float deepest=0;D3DXVECTOR3 push(0,0,0);
        Query(bounds,[&](const Triangle& tri) {
            D3DXVECTOR3 n;float penetration;
            if(BoxContact(center,half,yaw,tri,previous,n,penetration) && penetration>deepest)
            {deepest=penetration;push=n;}
        },ignore);
        if(deepest<=0) break;
        center+=push*(deepest+0.003f);
        contacts.ground=contacts.ground||push.y>0.55f;contacts.ceiling=contacts.ceiling||push.y< -0.55f;contacts.blocked=true;
    }
    return contacts;
}
}
#endif
