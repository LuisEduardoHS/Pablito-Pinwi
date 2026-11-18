#ifndef GC_SCENE_EFFECTS_H
#define GC_SCENE_EFFECTS_H
#include "MeshPrimitives.h"
#include "Gameplay/InteractionSystem.h"
#include <vector>

// Shared b3 material parameters. No changes to interaction rules or geometry.
class SceneEffects
{
    struct Data {D3DXVECTOR4 magic,highlight,clock;} data={};
    ID3D11PixelShader* shader=nullptr;ID3D11Buffer* buffer=nullptr;
    ID3D11DeviceContext* context=nullptr; // Borrowed from DXRender.
    struct Highlight {int id;float strength;};std::vector<Highlight> highlights;
    float seconds=0;
    template<typename T> static void Free(T*& p){if(p){p->Release();p=nullptr;}}
public:
    SceneEffects()=default;SceneEffects(const SceneEffects&)=delete;SceneEffects& operator=(const SceneEffects&)=delete;
    ~SceneEffects(){Release();}
    bool Init(ID3D11Device* device,ID3D11DeviceContext* deviceContext)
    {
        Release();context=deviceContext;ID3DBlob* code=nullptr;if(!CompileSceneShader(L"materialEffects.ps","PS","ps_4_0",&code))return false;
        HRESULT hr=device->CreatePixelShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,&shader);code->Release();
        D3D11_BUFFER_DESC desc={};desc.ByteWidth=sizeof(Data);desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        if(FAILED(hr) || FAILED(device->CreateBuffer(&desc,nullptr,&buffer))){Release();return false;}return true;
    }
    void Advance(float elapsed)
    {if(std::isfinite(elapsed) && elapsed>0)seconds+=elapsed;}
    float Time() const{return seconds;}
    void UpdateHighlights(const std::vector<Alchemy::Target>& targets,Alchemy::Position player,int selected)
    {
        highlights.clear();
        for(const auto& target:targets)
        {
            if(!target.visible || fabsf(target.position.y-player.y)>3.8f)continue;
            const float dx=target.position.x-player.x,dz=target.position.z-player.z,distance=sqrtf(dx*dx+dz*dz);
            const float range=target.range;if(range<=0 || distance>=range)continue;
            float t=(range-distance)/(range*.4f);t=(std::max)(0.0f,(std::min)(1.0f,t));t=t*t*(3-2*t);
            highlights.push_back({target.id,t*(target.id==selected?1.15f:.75f)});
        }
    }
    void Reset(){seconds=0;highlights.clear();}
    float HighlightStrength(int id) const
    {for(const auto& value:highlights)if(value.id==id)return value.strength;return 0;}
    static D3DXVECTOR3 PotionColor(int recipe)
    {const D3DXVECTOR3 colors[]={{1.0f,.58f,.12f},{.20f,.64f,1.0f},{.22f,1.0f,.55f}};return colors[recipe>=0?recipe%3:0];}
    void Draw(MPrimitives* mesh,Camara* camera,int targetId=-1,D3DXVECTOR3 emission=D3DXVECTOR3(0,0,0),float strength=0,float phase=0)
    {
        const float highlight=HighlightStrength(targetId);
        if(!shader || !context || (!highlight && !strength)){mesh->DrawScene(camera);return;}
        data.magic=D3DXVECTOR4(emission.x,emission.y,emission.z,strength);
        data.highlight=D3DXVECTOR4(.25f,.85f,1.0f,highlight);data.clock=D3DXVECTOR4(seconds,phase,0,0);
        context->UpdateSubresource(buffer,0,nullptr,&data,0,0);context->PSSetConstantBuffers(3,1,&buffer);
        mesh->DrawScene(camera,shader);
    }
    void Release(){Free(shader);Free(buffer);context=nullptr;highlights.clear();seconds=0;}
};
#endif
