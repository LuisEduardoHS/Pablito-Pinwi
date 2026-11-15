// CPU regression: compile x86 with the June 2010 DirectX SDK (d3dx10.lib).
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "../SceneCollision.h"
#pragma comment(lib,"d3dx10.lib")
using namespace SceneCollision;
static unsigned checks=0;
static void Check(bool value,const char* message)
{++checks;if(!value) {fprintf(stderr,"FAIL: %s\n",message);exit(1);}}
static bool Near(float a,float b,float epsilon=0.015f) {return fabsf(a-b)<epsilon;}
static void Quad(std::vector<Triangle>& t,D3DXVECTOR3 a,D3DXVECTOR3 b,D3DXVECTOR3 c,D3DXVECTOR3 d)
{t.emplace_back(a,b,c);t.emplace_back(a,c,d);}
static std::vector<Triangle> Wall(float x,float z0=-5,float z1=5,float y0=0,float y1=5)
{
    std::vector<Triangle> t;
    Quad(t,D3DXVECTOR3(x,y0,z0),D3DXVECTOR3(x,y1,z0),D3DXVECTOR3(x,y1,z1),D3DXVECTOR3(x,y0,z1));return t;
}
int main()
{
    Check(MeshCollider::All().empty(),"empty registry");
    {
        MeshCollider wall;wall.Build(Wall(0));
        Check(wall.TriangleCount()==2 && MeshCollider::All().size()==1,"build auto-registers");
        for(float speed : {8.0f,16.0f,28.0f,100.0f}) for(int fps : {15,60,144})
        {
            D3DXVECTOR3 p(-3,0,0);Contacts contacts;
            for(int i=0;i<fps;++i) p=MoveCapsule(p,D3DXVECTOR3(speed/fps,0,0),3,0.5f,contacts);
            Check(p.x<= -0.499f && p.x>= -0.52f,"thin wall stops high-speed capsule");
            Check(contacts.blocked,"wall contact reported");
        }
        Contacts contacts;
        D3DXVECTOR3 slide=MoveCapsule(D3DXVECTOR3(-0.6f,0,-2),D3DXVECTOR3(2,0,3),3,0.5f,contacts);
        Check(slide.x< -0.49f && slide.z>0.95f,"wall sliding preserves tangent motion");
        D3DXVECTOR3 back=MoveCapsule(D3DXVECTOR3(3,0,0),D3DXVECTOR3(-6,0,0),3,0.5f,contacts);
        Check(back.x>0.49f,"back face blocks too");
        D3DXVECTOR3 exact(0,0,0);ResolveCapsule(exact,3,0.5f,D3DXVECTOR3(-1,0,0));
        Check(exact.x< -0.49f,"exact plane contact chooses previous side");
        float hit;
        Check(Raycast(D3DXVECTOR3(-2,2,0),D3DXVECTOR3(2,2,0),hit) && Near(hit,0.5f),"camera/interaction ray");
        Check(!Raycast(D3DXVECTOR3(-2,2,0),D3DXVECTOR3(2,2,0),hit,&wall),"ignore own vehicle");
        for(float yaw : {-1.2f,0.4f,1.7f}) for(float width : {-2.0f,0.4f,3.0f})
        {
            D3DXMATRIX s,r,t;D3DXMatrixScaling(&s,width,1.5f,0.35f);D3DXMatrixRotationY(&r,yaw);D3DXMatrixTranslation(&t,15,2,25);
            wall.SetWorld(s*r*t);
            const D3DXVECTOR3 normal=RotateY(D3DXVECTOR3(1,0,0),yaw),origin(15,2,25);
            Contacts contact;const D3DXVECTOR3 result=MoveCapsule(origin-normal*3,normal*6,3,0.5f,contact);
            Check(Dot(result-origin,normal)<= -0.49f,"capsule narrow phase respects scaled/rotated BVH queries");
        }
        wall.SetEnabled(false);Check(CapsuleFree(D3DXVECTOR3(0,0,0),3,0.5f),"explicit opt-out");wall.SetEnabled(true);
        D3DXMATRIX scale,rotate,translate;
        D3DXMatrixScaling(&scale,-2,1.5f,0.3f);D3DXMatrixRotationY(&rotate,0.7f);D3DXMatrixTranslation(&translate,10,3,8);
        wall.SetWorld(scale*rotate*translate);
        D3DXVECTOR3 a=Transform(D3DXVECTOR3(-2,2,0),scale*rotate*translate),b=Transform(D3DXVECTOR3(2,2,0),scale*rotate*translate);
        Check(Raycast(a,b,hit) && Near(hit,0.5f),"rotation, translation, nonuniform and mirrored scale");
        D3DXMatrixScaling(&scale,0,0,0);wall.SetWorld(scale);Check(!wall.IsEnabled(),"singular transform excluded safely");
    }
    Check(MeshCollider::All().empty(),"destruction unregisters");
    {
        MeshCollider doorway;
        auto faces=Wall(0,-5,-1.3f);auto right=Wall(0,1.3f,5);
        faces.insert(faces.end(),right.begin(),right.end());
        auto lintel=Wall(0,-1.3f,1.3f,3.4f,5);faces.insert(faces.end(),lintel.begin(),lintel.end());doorway.Build(faces);
        Contacts contacts;
        D3DXVECTOR3 through=MoveCapsule(D3DXVECTOR3(-3,0,0),D3DXVECTOR3(6,0,0),3,0.5f,contacts);
        Check(Near(through.x,3),"door opening remains open");
        D3DXVECTOR3 side=MoveCapsule(D3DXVECTOR3(-3,0,2),D3DXVECTOR3(6,0,0),3,0.5f,contacts);
        Check(side.x< -0.49f,"doorframe still solid");
    }
    {
        MeshCollider table;std::vector<Triangle> faces;
        Quad(faces,D3DXVECTOR3(-2,1,-2),D3DXVECTOR3(2,1,-2),D3DXVECTOR3(2,1,2),D3DXVECTOR3(-2,1,2));table.Build(faces);
        Contacts contacts;
        D3DXVECTOR3 p=MoveCapsule(D3DXVECTOR3(0,5,0),D3DXVECTOR3(0,-8,0),3,0.5f,contacts);
        Check(Near(p.y,1) && contacts.ground,"fall lands on table");
        p=MoveCapsule(D3DXVECTOR3(0,-3,0),D3DXVECTOR3(0,4,0),3,0.5f,contacts);
        Check(p.y<= -1.99f && contacts.ceiling,"ceiling blocks ascent");
        Check(CapsuleFree(D3DXVECTOR3(0,0,0),0.5f,0.2f),"space below tabletop is not a solid bounding box");
    }
    {
        MeshCollider corner;auto faces=Wall(0);std::vector<Triangle> perpendicular;
        Quad(perpendicular,D3DXVECTOR3(-5,0,0),D3DXVECTOR3(5,0,0),D3DXVECTOR3(5,5,0),D3DXVECTOR3(-5,5,0));
        faces.insert(faces.end(),perpendicular.begin(),perpendicular.end());corner.Build(faces);
        Contacts hit;D3DXVECTOR3 p=MoveCapsule(D3DXVECTOR3(-2,0,-2),D3DXVECTOR3(4,0,4),3,0.5f,hit);
        Check(p.x< -0.49f && p.z< -0.49f,"inside corner does not leak");
    }
    {
        MeshCollider obstacle;obstacle.Build(Wall(0));
        const D3DXVECTOR3 half(2,2,2.5f);
        for(float yaw : {0.0f,0.3f,0.8f,1.57f})
        {
            D3DXVECTOR3 p(-4,2,0);
            for(int i=0;i<100;++i) {D3DXVECTOR3 previous=p;p.x+=0.14f;ResolveBox(p,half,yaw,previous);}
            const float extent=fabsf(cosf(yaw))*half.x+fabsf(sinf(yaw))*half.z;
            Check(p.x<= -extent+0.01f,"oriented vehicle box stops at wall");
        }
    }
    {
        MeshCollider dense;std::vector<Triangle> faces;
        for(int i=0;i<2000;++i) {auto w=Wall(static_cast<float>(i*10),-1,1);faces.insert(faces.end(),w.begin(),w.end());}
        dense.Build(faces);Bounds query;query.Add(D3DXVECTOR3(-1,0,-1));query.Add(D3DXVECTOR3(1,3,1));int candidates=0;
        dense.Query(query,[&](const Triangle&){++candidates;});
        Check(candidates>0 && candidates<20,"BVH prunes distant triangles");
        dense.Clear();Check(MeshCollider::All().empty(),"resource release unregisters");
    }
    printf("PASS: %u collision checks (sweeps, holes, support, ceilings, sliding, transforms, boxes, BVH and lifetime).\n",checks);
}
