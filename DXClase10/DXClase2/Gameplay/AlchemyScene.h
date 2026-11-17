#ifndef GC_ALCHEMY_SCENE_H
#define GC_ALCHEMY_SCENE_H
#include "AlchemyGame.h"
#include "../MeshPrimitives.h"

namespace Alchemy
{
class Scene
{
    struct Prop {MPrimitives* mesh;D3DXMATRIX world;};
    std::vector<Prop> props;
    std::array<MPrimitives*,6> ingredients{{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}};
    std::array<const SceneCollision::MeshCollider*,4> stationColliders{{nullptr,nullptr,nullptr,nullptr}};
    std::vector<const SceneCollision::MeshCollider*> deliveryColliders;
    MPrimitives *bottle=nullptr,*liquid=nullptr,*fire=nullptr;
    int lastColor=-1;
    std::vector<MPrimitives*> owned;
    ID3D11Device* device=nullptr;ID3D11DeviceContext* context=nullptr;
    MPrimitives* Own(MPrimitives* mesh) {owned.push_back(mesh);return mesh;}
    MPrimitives* Cube(unsigned char r,unsigned char g,unsigned char b)
    {auto* p=Own(new MPrimitives(device,context,1.0f,L"",L"",ShaderDiffuse));if(!p->SetSolidColor(r,g,b)) valid=false;return p;}
    MPrimitives* Cylinder(float radius,float height,unsigned char r,unsigned char g,unsigned char b)
    {auto* p=Own(new MPrimitives(device,context,20,radius,radius,height,L"",L"",ShaderDiffuse));if(!p->SetSolidColor(r,g,b)) valid=false;return p;}
    static D3DXMATRIX Matrix(Position p,Position size=Position(1,1,1),float yaw=0,float pitch=0)
    {
        D3DXMATRIX s,r,t;D3DXMatrixScaling(&s,size.x,size.y,size.z);D3DXMatrixRotationYawPitchRoll(&r,yaw,pitch,0);
        D3DXMatrixTranslation(&t,p.x,p.y,p.z);return s*r*t;
    }
    void Place(MPrimitives* mesh,Position p,Position scale=Position(1,1,1),float pitch=0)
    {D3DXMATRIX world=Matrix(p,scale,0,pitch);mesh->UpdateScene(world);props.push_back({mesh,world});}
    MPrimitives* Table(float x,float z,float floor)
    {
        auto* mesh=Own(new MPrimitives(device,context,"Models/PablitoPinwi/Mesa.txt",L"Models/PablitoPinwi/Mesa.png",L"",ShaderDiffuse));
        if(!mesh->IsModelLoaded())valid=false;Place(mesh,{x,floor,z},{4,4,4});return mesh;
    }
    MPrimitives* ItemMesh(const Item& item) const
    {return static_cast<int>(item.type)<6?ingredients[static_cast<int>(item.type)]:bottle;}
    void PoseItem(const Item& item) const
    {
        const float size=static_cast<int>(item.type)<6?1.0f:4.0f;
        ItemMesh(item)->UpdateScene(Matrix(item.position,{size,size,size},item.yaw));
    }
public:
    bool valid=true;
    Scene()=default;Scene(const Scene&)=delete;Scene& operator=(const Scene&)=delete;
    ~Scene(){Release();}
    bool Init(ID3D11Device* d,ID3D11DeviceContext* c,MPrimitives* terrain,MPrimitives* existingBottle,
        MPrimitives* existingFurnace,Game& game)
    {
        Release();device=d;context=c;valid=true;lastColor=-1;bottle=existingBottle;bottle->SetCollisionEnabled(false);
        auto floor=[&](float x,float z){return terrain->getTerrainCollision(D3DXVECTOR2(x,z));};
        auto* supplyA=Table(375,399,floor(375,399));auto* supplyB=Table(375,407,floor(375,407));
        for(int i=0;i<6;++i)
        {float z=i<3?399.0f:407.0f;game.layout.sources[i]=Position(374.0f+i%3,floor(375,z)+(i<3?supplyA:supplyB)->GetModelHeight()*4+0.45f,z);}
        game.layout.bottle=Position(384,floor(384,403)+supplyA->GetModelHeight()*4,403);
        auto* mortarTable=Table(390,395,floor(390,395));
        auto* mortar=Cylinder(0.65f,0.48f,117,107,139);
        Position mortarPos(390,floor(390,395)+mortarTable->GetModelHeight()*4+0.24f,395);Place(mortar,mortarPos);
        game.layout.stations[0]=Position(390,mortarPos.y+0.30f,395);stationColliders[0]=mortar->GetCollider();
        auto* cauldron=Cylinder(1.35f,1.7f,58,50,79);Place(cauldron,{401,floor(401,392)+1.25f,392});stationColliders[1]=cauldron->GetCollider();
        auto* rim=Own(new MPrimitives(d,c,16,24,1.30f,0.14f,L"",L"",ShaderDiffuse));if(!rim->SetSolidColor(150,126,182))valid=false;
        Place(rim,{401,floor(401,392)+2.12f,392},{1,1,1},D3DX_PI*0.5f);
        for(float x:{-0.8f,0.8f}) Place(Cube(58,50,79),{401+x,floor(401,392)+0.30f,392},{0.35f,0.6f,0.7f});
        liquid=Cylinder(1.15f,0.08f,78,67,114);liquid->SetCollisionEnabled(false);Place(liquid,{401,floor(401,392)+2.10f,392});
        game.layout.stations[1]=Position(401,floor(401,392)+2.3f,392);
        game.layout.stations[2]=Position(394,floor(394,412)+1.5f,412);stationColliders[2]=existingFurnace->GetCollider();
        fire=Own(new MPrimitives(d,c,12,16,0.38f,L"",L"",ShaderDiffuse));if(!fire->SetSolidColor(255,149,51))valid=false;
        fire->SetCollisionEnabled(false);Place(fire,{394,floor(394,412)+0.8f,412});
        auto* bottlerTable=Table(390,407,floor(390,407));stationColliders[3]=bottlerTable->GetCollider();
        const float top=floor(390,407)+bottlerTable->GetModelHeight()*4;
        Place(Cylinder(0.12f,1.3f,162,159,177),{390.9f,top+0.65f,407});
        Place(Cube(162,159,177),{390.3f,top+1.25f,407},{1.3f,0.18f,0.25f});
        game.layout.stations[3]=Position(390,top,407);
        const unsigned char colors[6][3]={{213,64,72},{72,139,234},{167,101,231},{74,200,222},{92,185,97},{244,199,80}};
        for(int i=0;i<6;++i)
        {
            MPrimitives* mesh;
            if(i==2)mesh=Own(new MPrimitives(d,c,12,0.0f,0.32f,0.95f,L"",L"",ShaderDiffuse));
            else if(i==4)mesh=Cube(colors[i][0],colors[i][1],colors[i][2]);
            else if(i==5)mesh=Cylinder(0.16f,0.85f,colors[i][0],colors[i][1],colors[i][2]);
            else mesh=Own(new MPrimitives(d,c,12,16,0.37f,L"",L"",ShaderDiffuse));
            if(!mesh->SetSolidColor(colors[i][0],colors[i][1],colors[i][2])) valid=false;
            mesh->SetCollisionEnabled(false);ingredients[i]=mesh;
        }
        const unsigned char signColors[3][3]={{238,184,68},{75,153,238},{84,190,118}};
        deliveryColliders.resize(game.layout.deliveries.size(),nullptr);
        for(size_t i=0;i<game.layout.deliveries.size();++i)
        {
            Position p=game.layout.deliveries[i];p.y=floor(p.x,p.z);
            auto* pole=Cylinder(0.15f,2.8f,116,88,62);Place(pole,{p.x,p.y+1.4f,p.z});deliveryColliders[i]=pole->GetCollider();
            Place(Cube(signColors[i%3][0],signColors[i%3][1],signColors[i%3][2]),{p.x,p.y+2.55f,p.z},{1.5f,0.8f,0.25f});
            game.layout.deliveries[i]=Position(p.x,p.y+1.5f,p.z);
        }
        game.Restart();return valid;
    }
    const SceneCollision::MeshCollider* IgnoreFor(const Target& target) const
    {
        if(target.kind==TargetKind::Delivery)return deliveryColliders[target.index];
        if(target.kind==TargetKind::Mortar || target.kind==TargetKind::Cauldron || target.kind==TargetKind::Hearth || target.kind==TargetKind::Bottler)
            return stationColliders[target.index];
        return nullptr;
    }
    void Sync(const Game& game)
    {
        const int color=static_cast<int>(game.batchState)+(game.heating?20:0);
        if(color!=lastColor && liquid)
        {
            if(game.batchState==BatchState::Invalid)liquid->SetSolidColor(218,68,82);
            else if(game.heating)liquid->SetSolidColor(246,151,64);
            else if(game.batchState==BatchState::Mixed || game.batchState==BatchState::Heated)liquid->SetSolidColor(80,210,151);
            else if(game.batchState==BatchState::Mixing)liquid->SetSolidColor(91,156,231);
            else liquid->SetSolidColor(112,88,161);
            lastColor=color;
        }
    }
    void Draw(const Game& game,Camara* camera)
    {
        for(const Prop& prop:props) {if(prop.mesh==fire && !game.heating)continue;prop.mesh->UpdateScene(prop.world);prop.mesh->DrawScene(camera);}
        for(const Item& item:game.items) if(item.state!=ItemState::Consumed) {PoseItem(item);ItemMesh(item)->DrawScene(camera);}
    }
    void DrawShadow(const Game& game,const D3DXMATRIX& viewProjection)
    {
        for(const Prop& prop:props) {if(prop.mesh==fire || prop.mesh==liquid)continue;prop.mesh->UpdateScene(prop.world);prop.mesh->DrawShadow(viewProjection);}
        for(const Item& item:game.items) if(item.state!=ItemState::Consumed) {PoseItem(item);ItemMesh(item)->DrawShadow(viewProjection);}
    }
    void Release()
    {
        for(auto* mesh:owned) {mesh->ReleaseModelResources();delete mesh;}
        owned.clear();props.clear();ingredients.fill(nullptr);stationColliders.fill(nullptr);deliveryColliders.clear();bottle=liquid=fire=nullptr;
    }
};
}
#endif
