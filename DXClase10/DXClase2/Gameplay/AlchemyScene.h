#ifndef GC_ALCHEMY_SCENE_H
#define GC_ALCHEMY_SCENE_H
#include "AlchemyGame.h"
#include "../MeshPrimitives.h"
#include "../SceneEffects.h"
#include "../WorldLayout.h"

namespace Alchemy
{
class Scene
{
    struct Prop {MPrimitives* mesh;D3DXMATRIX world;int target;};
    int propTarget=-1;
    std::vector<Prop> props;
    std::array<MPrimitives*,6> ingredients{{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}};
    std::array<float,6> ingredientScales{{1,1,1,1,1,1}},ingredientHeights{{0,0,0,0,0,0}};
    std::array<const SceneCollision::MeshCollider*,4> stationColliders{{nullptr,nullptr,nullptr,nullptr}};
    std::vector<const SceneCollision::MeshCollider*> deliveryColliders;
    std::vector<MPrimitives*> deliveryBillboards;
    std::vector<Position> deliverySignPositions;
    float billboardAge = 0.0f;
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
    static Position HousePosition(float x,float y,float z)
    {const auto p=WorldLayout::HousePoint(x,y,z);return {p.x,p.y,p.z};}
    void Place(MPrimitives* mesh,Position p,Position scale=Position(1,1,1),float pitch=0,float yaw=0)
    {D3DXMATRIX world=Matrix(p,scale,yaw,pitch);mesh->UpdateScene(world);props.push_back({mesh,world,propTarget});}
    MPrimitives* Table(float x,float z)
    {
        auto* mesh=Own(new MPrimitives(device,context,"Models/PablitoPinwi/Mesa.txt",L"Models/PablitoPinwi/Mesa.png",L"",ShaderDiffuse));
        if(!mesh->IsModelLoaded())valid=false;Place(mesh,HousePosition(x,.03f,z),{4,4,4},0,WorldLayout::HouseYaw());return mesh;
    }
    void Bridge()
    {
        propTarget=-1;const float pitch=-atanf(WorldLayout::BridgeSlope),yaw=WorldLayout::BridgeYaw();
        const unsigned planks=30;const float length=WorldLayout::BridgeLength/planks;
        for(unsigned i=0;i<planks;++i)
        {auto p=WorldLayout::BridgePoint(-WorldLayout::BridgeLength*.5f+(i+.5f)*length);Place(Cube(143+(i%3)*5,99+(i%3)*3,62+(i%3)*2),{p.x,p.y,p.z},{WorldLayout::BridgeWidth,.35f,length*sqrtf(1+WorldLayout::BridgeSlope*WorldLayout::BridgeSlope)},pitch,yaw);}
        for(float side:{-WorldLayout::BridgeWidth*.48f,WorldLayout::BridgeWidth*.48f})
        {
            auto rail=WorldLayout::BridgePoint(0,side,1.35f);Place(Cube(114,76,47),{rail.x,rail.y,rail.z},{.20f,.20f,WorldLayout::BridgeLength},pitch,yaw);
            for(float along=-WorldLayout::BridgeLength*.5f;along<=WorldLayout::BridgeLength*.5f;along+=10)
            {auto p=WorldLayout::BridgePoint(along,side,.75f);Place(Cube(114,76,47),{p.x,p.y,p.z},{.3f,1.5f,.3f},0,yaw);}
        }
    }
    MPrimitives* ItemMesh(const Item& item) const
    {return static_cast<int>(item.type)<6?ingredients[static_cast<int>(item.type)]:bottle;}
    void PoseItem(const Item& item) const
    {
        const unsigned type=static_cast<unsigned>(item.type);const bool ingredient=type<ingredients.size();
        const float size=ingredient?ingredientScales[type]:4.0f;Position position=item.position;
        // Available/dropped/placed meshes rest on their base. While carried,
        // center their body at the existing grip without moving logical targets.
        if(ingredient && item.state==ItemState::Carried)position.y-=ingredientHeights[type]*.45f;
        // Show the root's length across the table/hands, rather than end-on.
        const float yaw=item.yaw+(item.type==Ingredient::Root?D3DX_PI*.5f:0);
        ItemMesh(item)->UpdateScene(Matrix(position,{size,size,size},yaw));
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
        // Interior footing above the terrain, contained by the existing walls.
        Place(Cube(133,100,70),HousePosition(7,.015f,0),{30,.10f,14},0,WorldLayout::HouseYaw());
        auto* supplyA=Table(-4,-5);auto* supplyB=Table(-4,5);
        for(int i=0;i<6;++i)
        {float z=i<3?-5.0f:5.0f;game.layout.sources[i]=HousePosition(-5.0f+i%3,.03f+(i<3?supplyA:supplyB)->GetModelHeight()*4,z);}
        game.layout.bottle=HousePosition(9,.03f+supplyA->GetModelHeight()*4,-5);
        propTarget=10;auto* mortarTable=Table(4,-6);
        auto* mortar=Cylinder(0.65f,0.48f,117,107,139);
        Position mortarPos=HousePosition(4,.03f+mortarTable->GetModelHeight()*4+.24f,-6);Place(mortar,mortarPos);
        game.layout.stations[0]=Position(mortarPos.x,mortarPos.y+.30f,mortarPos.z);stationColliders[0]=mortar->GetCollider();
        propTarget=11;auto* cauldron=Cylinder(1.35f,1.7f,58,50,79);Place(cauldron,HousePosition(16,1.28f,5));stationColliders[1]=cauldron->GetCollider();
        auto* rim=Own(new MPrimitives(d,c,16,24,1.30f,0.14f,L"",L"",ShaderDiffuse));if(!rim->SetSolidColor(150,126,182))valid=false;
        Place(rim,HousePosition(16,2.15f,5),{1,1,1},D3DX_PI*0.5f);
        for(float x:{-.8f,.8f})Place(Cube(58,50,79),HousePosition(16+x,.33f,5),{.35f,.6f,.7f},0,WorldLayout::HouseYaw());
        liquid=Cylinder(1.15f,0.08f,78,67,114);liquid->SetCollisionEnabled(false);Place(liquid,HousePosition(16,2.13f,5));
        game.layout.stations[1]=HousePosition(16,2.33f,5);
        game.layout.stations[2]=HousePosition(7,1.53f,6);stationColliders[2]=existingFurnace->GetCollider();
        propTarget=12;fire=Own(new MPrimitives(d,c,12,16,0.38f,L"",L"",ShaderDiffuse));if(!fire->SetSolidColor(255,149,51))valid=false;
        fire->SetCollisionEnabled(false);Place(fire,HousePosition(7,.83f,6));
        propTarget=13;auto* bottlerTable=Table(15,-4);stationColliders[3]=bottlerTable->GetCollider();
        const float top=.03f+bottlerTable->GetModelHeight()*4;
        Place(Cylinder(.12f,1.3f,162,159,177),HousePosition(15.9f,top+.65f,-4));
        Place(Cube(162,159,177),HousePosition(15.3f,top+1.25f,-4),{1.3f,.18f,.25f},0,WorldLayout::HouseYaw());
        game.layout.stations[3]=HousePosition(15,top,-4);
        const char* ingredientModelPaths[6] = {
            "Models/ingredientes/hongorojo.txt",
            "Models/ingredientes/florazul.txt",
            "Models/ingredientes/cristal.txt",
            "Models/ingredientes/agua.txt",
            "Models/ingredientes/hierba.txt",
            "Models/ingredientes/raiz.txt"
        };
        // Longest dimension in world units; uniform scaling retains the art's
        // proportions and leaves a gap between the one-unit supply positions.
        const float extents[6]={.82f,.80f,.86f,.78f,.76f,.84f};
        for(int i=0;i<6;++i)
        {
            auto* mesh=Own(new MPrimitives(d,c,ingredientModelPaths[i],L"Texturas/paleta.png",L"",ShaderDiffuse,true));
            if(!mesh->IsModelLoaded()) valid=false;
            const auto size=mesh->GetModelSize();const float longest=(std::max)(size.x,(std::max)(size.y,size.z));
            if(!std::isfinite(longest) || longest<=0){valid=false;continue;}
            ingredientScales[i]=extents[i]/longest;ingredientHeights[i]=size.y*ingredientScales[i];
            mesh->SetCollisionEnabled(false);
            ingredients[i]=mesh;
        }
        const unsigned char signColors[3][3]={{238,184,68},{75,153,238},{84,190,118}};
        game.layout.deliveries.clear();
        for(const auto& house:WorldLayout::SecondaryHouses)
        {game.layout.deliveries.push_back({house.delivery.x,house.delivery.y,house.delivery.z});}
        const wchar_t* houseSignTextures[3] = {
            L"Texturas/CartelesCasas/vecina.jpeg",
            L"Texturas/CartelesCasas/mensajero.jpeg",
            L"Texturas/CartelesCasas/guardabosques.jpeg"
        };
        deliveryColliders.resize(game.layout.deliveries.size(),nullptr);
        deliveryBillboards.resize(game.layout.deliveries.size(),nullptr);
        deliverySignPositions.resize(game.layout.deliveries.size());
        for(size_t i=0;i<game.layout.deliveries.size();++i)
        {
            propTarget=-1;const auto& house=WorldLayout::SecondaryHouses[i];
            auto* dwelling=Own(new MPrimitives(d,c,"Models/PablitoPinwi/casa_pablo.txt",L"Models/PablitoPinwi/casa.png",L"",ShaderDiffuse,true));
            if(!dwelling->IsModelLoaded())valid=false;
            Place(dwelling,{house.center.x,house.center.y,house.center.z},{2.2f,2.2f,2.2f},0,D3DXToRadian(house.yawDegrees));
            propTarget=20+static_cast<int>(i);
            Position p=game.layout.deliveries[i];p.y=floor(p.x,p.z);
            auto* pole=Cylinder(0.15f,2.8f,116,88,62);Place(pole,{p.x,p.y+1.4f,p.z});deliveryColliders[i]=pole->GetCollider();
            Place(Cube(signColors[i%3][0],signColors[i%3][1],signColors[i%3][2]),{p.x,p.y+2.55f,p.z},{1.5f,0.8f,0.25f});
            game.layout.deliveries[i]=Position(p.x,p.y+1.5f,p.z);
            deliverySignPositions[i]=p;

            auto* bb=Own(new MPrimitives(d,c,2.0f,1.6f,houseSignTextures[i%3]));
            bb->SetCollisionEnabled(false);
            deliveryBillboards[i]=bb;
        }
        Bridge();game.Restart();return valid;
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
    void Draw(const Game& game,Camara* camera,SceneEffects& effects)
    {
        for(const Prop& prop:props)
        {
            if(prop.mesh==fire && !game.heating)continue;prop.mesh->UpdateScene(prop.world);
            const bool magical=prop.mesh==liquid && (game.batchState==BatchState::Mixed || game.batchState==BatchState::Heated);
            effects.Draw(prop.mesh,camera,prop.mesh==fire?-1:prop.target,SceneEffects::PotionColor(game.CurrentRecipe().id),magical?.65f:0);
        }
        for(const Item& item:game.items)if(item.state!=ItemState::Consumed)
        {
            PoseItem(item);const bool potion=item.type==Ingredient::Potion;
            const int target=item.station>=0?10+item.station:1000+item.id;
            effects.Draw(ItemMesh(item),camera,target,item.correct?SceneEffects::PotionColor(item.recipe):D3DXVECTOR3(1,.22f,.42f),potion?1.1f:0,item.id*.63f);
        }
        billboardAge += 0.016f;
        for(size_t i=0;i<deliveryBillboards.size();++i)
        {
            if(!deliveryBillboards[i]) continue;
            const Position p = deliverySignPositions[i];
            const float floatHeight = p.y + 4.2f + 0.12f * sinf(billboardAge * 2.5f + static_cast<float>(i) * 2.0f);
            D3DXMATRIX w;
            D3DXMatrixTranslation(&w, p.x, floatHeight, p.z);
            deliveryBillboards[i]->UpdateScene(w);
            effects.Draw(deliveryBillboards[i], camera, 20 + static_cast<int>(i));
        }
    }
    void DrawShadow(const Game& game,const D3DXMATRIX& viewProjection)
    {
        for(const Prop& prop:props) {if(prop.mesh==fire || prop.mesh==liquid)continue;prop.mesh->UpdateScene(prop.world);prop.mesh->DrawShadow(viewProjection);}
        for(const Item& item:game.items) if(item.state!=ItemState::Consumed) {PoseItem(item);ItemMesh(item)->DrawShadow(viewProjection);}
    }
    void Release()
    {
        for(auto* mesh:owned) {mesh->ReleaseModelResources();delete mesh;}
        owned.clear();props.clear();propTarget=-1;ingredients.fill(nullptr);ingredientScales.fill(1);ingredientHeights.fill(0);stationColliders.fill(nullptr);deliveryColliders.clear();
        deliveryBillboards.clear();deliverySignPositions.clear();billboardAge=0.0f;bottle=liquid=fire=nullptr;
    }
};
}
#endif
