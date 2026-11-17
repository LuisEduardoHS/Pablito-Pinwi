#ifndef GC_VEGETATION_H
#define GC_VEGETATION_H

#include "MeshPrimitives.h"
#include "Camara.h"
#include <cstdint>
#include <cstring>

// Reproducible meadow billboards. No physics colliders or per-plant draws.
class Vegetation
{
public:
    struct Plant
    {
        D3DXVECTOR3 position;D3DXVECTOR2 size;D3DXVECTOR3 tint;
        float mirror;D3DXVECTOR3 normal;D3DXVECTOR2 slope;
    };
    const std::vector<Plant>& Plants(unsigned type) const {return plants[type];}
    unsigned VisibleCount() const {return visibleCount;}
    Vegetation()=default;Vegetation(const Vegetation&)=delete;Vegetation& operator=(const Vegetation&)=delete;
    ~Vegetation(){Release();}
private:
    std::vector<Plant> plants[3],visible;
    ID3D11ShaderResourceView* textures[3]={};
    ID3D11VertexShader* vs=nullptr;ID3D11PixelShader* ps=nullptr;ID3D11InputLayout* layout=nullptr;
    ID3D11Buffer* instances=nullptr;ID3D11Buffer* constants=nullptr;
    ID3D11SamplerState* sampler=nullptr;ID3D11DepthStencilState* depth=nullptr;ID3D11RasterizerState* raster=nullptr;
    unsigned visibleCount=0;
    struct ViewData {D3DXMATRIX viewProjection;D3DXVECTOR4 right,camera,bounds;};
    // Original transparent padding is skipped with UVs; source PNGs stay intact.
    static D3DXVECTOR4 ImageBounds(unsigned type)
    {
        const float boxes[3][4]={{80,596,1321,1080},{1,518,1342,1080},{22,535,1317,1080}};
        return D3DXVECTOR4(boxes[type][0]/1350,boxes[type][1]/1080,boxes[type][2]/1350,1);
    }
    template<typename T> static void Free(T*& value){if(value){value->Release();value=nullptr;}}
    struct Random
    {
        uint32_t state=0x50494e57u;
        float Next(){state^=state<<13;state^=state>>17;state^=state<<5;return (state>>8)*(1.0f/16777216.0f);}
    };
    struct Mask
    {
        unsigned width=0,height=0;std::vector<unsigned char> pixels;
        bool Load(ID3D11Device* device,ID3D11DeviceContext* context,const std::wstring& path)
        {
            D3DX11_IMAGE_LOAD_INFO info;info.MipLevels=1;info.Usage=D3D11_USAGE_STAGING;
            info.BindFlags=0;info.CpuAccessFlags=D3D11_CPU_ACCESS_READ;info.MiscFlags=0;info.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
            ID3D11Resource* resource=nullptr;
            if(FAILED(D3DX11CreateTextureFromFile(device,path.c_str(),&info,nullptr,&resource,nullptr)))return false;
            ID3D11Texture2D* texture=nullptr;HRESULT hr=resource->QueryInterface(__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&texture));resource->Release();
            if(FAILED(hr))return false;
            D3D11_TEXTURE2D_DESC desc={};texture->GetDesc(&desc);width=desc.Width;height=desc.Height;
            D3D11_MAPPED_SUBRESOURCE mapped={};hr=context->Map(texture,0,D3D11_MAP_READ,0,&mapped);
            if(SUCCEEDED(hr))
            {
                pixels.resize(static_cast<size_t>(width)*height*4);
                for(unsigned y=0;y<height;++y)std::memcpy(pixels.data()+y*width*4,static_cast<unsigned char*>(mapped.pData)+y*mapped.RowPitch,width*4);
                context->Unmap(texture,0);
            }
            texture->Release();return SUCCEEDED(hr);
        }
        float Channel(float u,float v,unsigned channel) const
        {
            // Match D3D's linear, clamped texture sampling and the terrain's Z flip.
            const float x=u*width-0.5f,y=v*height-0.5f;
            const int ix=static_cast<int>(floorf(x)),iy=static_cast<int>(floorf(y));const float fx=x-ix,fy=y-iy;
            auto value=[&](int a,int b){a=(std::max)(0,(std::min)(static_cast<int>(width)-1,a));b=(std::max)(0,(std::min)(static_cast<int>(height)-1,b));return pixels[(b*width+a)*4+channel]/255.0f;};
            return (value(ix,iy)*(1-fx)+value(ix+1,iy)*fx)*(1-fy)+(value(ix,iy+1)*(1-fx)+value(ix+1,iy+1)*fx)*fy;
        }
    };
    bool Scatter(MPrimitives& terrain,const TerrainSettings& settings,const Mask& mask)
    {
        const float width=settings.width*settings.horizontalScale,depth=settings.depth*settings.horizontalScale;
        const D3DXMATRIX world=terrain.getWorldMatrix();
        const float cell=3.5f;const int columns=static_cast<int>(ceilf(width/cell)),rows=static_cast<int>(ceilf(depth/cell));
        struct Site {D3DXVECTOR3 position;float radius;unsigned type;};
        std::vector<Site> sites;std::vector<std::vector<size_t>> buckets(columns*rows);Random random;
        std::vector<SceneCollision::Bounds> obstacles;
        for(const auto* collider:SceneCollision::MeshCollider::All())if(collider->IsEnabled())obstacles.push_back(collider->GetWorldBounds());
        auto grass=[&](float x,float z,D3DXVECTOR3& position,D3DXVECTOR3& normal)
        {
            const float u=(x-world._41)/width,v=1-(z-world._43)/depth;
            return u>0 && u<1 && v>0 && v<1 && mask.Channel(u,v,0)<0.025f && mask.Channel(u,v,1)<0.18f
                && terrain.SampleTerrainSurface(D3DXVECTOR2(x,z),position,normal) && normal.y>0.82f;
        };
        for(int z=0;z<rows;++z)for(int x=0;x<columns;++x)
        {
            const float px=world._41+(x+0.05f+random.Next()*0.9f)*cell,pz=world._43+(z+0.05f+random.Next()*0.9f)*cell;
            // Broad, irregular patches with breathing room instead of a uniform grid.
            const float density=0.90f+0.10f*sinf(px*0.019f+0.8f*sinf(pz*0.011f))*cosf(pz*0.016f-px*0.006f);
            if(random.Next()>density)continue;
            const float choice=random.Next();const unsigned type=choice<0.50f?0:choice<0.90f?1:2;
            const float size=type==0?2.1f+random.Next()*1.2f:type==1?2.4f+random.Next()*1.5f:3.6f+random.Next()*1.4f;
            const float radius=size*0.55f;D3DXVECTOR3 position,normal;
            if(!grass(px,pz,position,normal))continue;
            bool clear=true;
            for(const auto& b:obstacles)
                if(px+radius+0.8f>b.low.x && px-radius-0.8f<b.high.x && pz+radius+0.8f>b.low.z && pz-radius-0.8f<b.high.z
                    && position.y+2.5f>b.low.y && position.y<b.high.y+0.3f){clear=false;break;}
            if(!clear)continue;
            const D3DXVECTOR2 slope(-normal.x/normal.y,-normal.z/normal.y);
            // Check the entire possible billboard footprint, not just its center.
            for(int i=0;i<8 && clear;++i)
            {
                const float angle=i*D3DX_PI/4,dx=cosf(angle)*radius,dz=sinf(angle)*radius;D3DXVECTOR3 sample,n;
                clear=grass(px+dx,pz+dz,sample,n) && fabsf(sample.y-position.y-slope.x*dx-slope.y*dz)<0.22f;
            }
            if(!clear)continue;
            for(int bz=(std::max)(0,z-3);bz<=(std::min)(rows-1,z+3) && clear;++bz)
            for(int bx=(std::max)(0,x-3);bx<=(std::min)(columns-1,x+3) && clear;++bx)
            for(size_t index:buckets[bz*columns+bx])
            {
                const Site& other=sites[index];const float dx=px-other.position.x,dz=pz-other.position.z;
                const float gap=type==2 && other.type==2?10.5f:radius+other.radius+((type==2 || other.type==2)?1.1f:0.45f);
                if(dx*dx+dz*dz<gap*gap){clear=false;break;}
            }
            if(!clear)continue;
            Plant plant;plant.position=position;plant.position.y-=0.06f;plant.normal=normal;plant.slope=slope;
            const D3DXVECTOR4 uv=ImageBounds(type);
            plant.size=D3DXVECTOR2(size,size*(uv.w-uv.y)*1080/((uv.z-uv.x)*1350));
            const float tone=0.76f+random.Next()*0.16f;
            plant.tint=D3DXVECTOR3(tone*(0.91f+random.Next()*0.07f),tone,tone*(0.90f+random.Next()*0.08f));
            plant.mirror=random.Next()<0.5f?-1.0f:1.0f;
            buckets[z*columns+x].push_back(sites.size());sites.push_back({position,radius,type});plants[type].push_back(plant);
        }
        return !sites.empty();
    }
public:
    bool Init(ID3D11Device* device,ID3D11DeviceContext* context,MPrimitives& terrain,const TerrainSettings& settings)
    {
        Release();Mask mask;if(!mask.Load(device,context,settings.blendMapPath) || !Scatter(terrain,settings,mask))return false;
        const wchar_t* files[]={L"Texturas/Vegetacion/pastito-noflores.png",L"Texturas/Vegetacion/pastito-noflores2.png",L"Texturas/Vegetacion/arbusto-flores.png"};
        for(unsigned i=0;i<3;++i)if(FAILED(D3DX11CreateShaderResourceViewFromFile(device,files[i],nullptr,nullptr,&textures[i],nullptr))){Release();return false;}
        ID3DBlob* vsCode=nullptr;ID3DBlob* psCode=nullptr;
        bool ok=CompileSceneShader(L"vegetation.hlsl","VS","vs_4_0",&vsCode) && CompileSceneShader(L"vegetation.hlsl","PS","ps_4_0",&psCode);
        const D3D11_INPUT_ELEMENT_DESC elements[]={
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_INSTANCE_DATA,1},
            {"SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_INSTANCE_DATA,1},
            {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_INSTANCE_DATA,1},
            {"TEXCOORD",0,DXGI_FORMAT_R32_FLOAT,0,32,D3D11_INPUT_PER_INSTANCE_DATA,1},
            {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,36,D3D11_INPUT_PER_INSTANCE_DATA,1},
            {"TEXCOORD",1,DXGI_FORMAT_R32G32_FLOAT,0,48,D3D11_INPUT_PER_INSTANCE_DATA,1}};
        if(ok)ok=SUCCEEDED(device->CreateVertexShader(vsCode->GetBufferPointer(),vsCode->GetBufferSize(),nullptr,&vs))
            && SUCCEEDED(device->CreatePixelShader(psCode->GetBufferPointer(),psCode->GetBufferSize(),nullptr,&ps))
            && SUCCEEDED(device->CreateInputLayout(elements,6,vsCode->GetBufferPointer(),vsCode->GetBufferSize(),&layout));
        Free(vsCode);Free(psCode);if(!ok){Release();return false;}
        unsigned total=0;for(const auto& group:plants)total+=static_cast<unsigned>(group.size());visible.reserve(total);
        D3D11_BUFFER_DESC buffer={};buffer.ByteWidth=total*sizeof(Plant);buffer.Usage=D3D11_USAGE_DYNAMIC;buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER;buffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
        if(FAILED(device->CreateBuffer(&buffer,nullptr,&instances))){Release();return false;}
        buffer={};buffer.ByteWidth=sizeof(ViewData);buffer.Usage=D3D11_USAGE_DEFAULT;buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        D3D11_SAMPLER_DESC sample={};sample.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sample.AddressU=sample.AddressV=sample.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sample.MaxLOD=D3D11_FLOAT32_MAX;
        D3D11_DEPTH_STENCIL_DESC ds={};ds.DepthEnable=TRUE;ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;ds.DepthFunc=D3D11_COMPARISON_LESS_EQUAL;
        D3D11_RASTERIZER_DESC rs={};rs.FillMode=D3D11_FILL_SOLID;rs.CullMode=D3D11_CULL_NONE;rs.DepthClipEnable=TRUE;
        if(FAILED(device->CreateBuffer(&buffer,nullptr,&constants)) || FAILED(device->CreateSamplerState(&sample,&sampler))
            || FAILED(device->CreateDepthStencilState(&ds,&depth)) || FAILED(device->CreateRasterizerState(&rs,&raster))){Release();return false;}
        return true;
    }
    void Draw(ID3D11DeviceContext* context,const Camara& camera)
    {
        if(!instances)return;visible.clear();unsigned counts[3]={};
        const D3DXMATRIX vp=camera.View*camera.Projection;
        for(unsigned type=0;type<3;++type)for(const Plant& plant:plants[type])
        {
            const D3DXVECTOR3 delta=plant.position-camera.Position;if(D3DXVec3LengthSq(&delta)>351*351)continue;
            D3DXVECTOR4 center(plant.position.x,plant.position.y+plant.size.y*0.5f,plant.position.z,1),clip;
            D3DXVec4Transform(&clip,&center,&vp);const float margin=plant.size.x*2;
            if(clip.w< -margin || clip.z< -margin || clip.z>clip.w+margin || fabsf(clip.x)>clip.w+margin || fabsf(clip.y)>clip.w+margin)continue;
            visible.push_back(plant);++counts[type];
        }
        visibleCount=static_cast<unsigned>(visible.size());if(!visibleCount)return;
        D3D11_MAPPED_SUBRESOURCE mapped={};if(FAILED(context->Map(instances,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return;
        std::memcpy(mapped.pData,visible.data(),visible.size()*sizeof(Plant));context->Unmap(instances,0);
        ID3D11BlendState* oldBlend=nullptr;ID3D11DepthStencilState* oldDepth=nullptr;ID3D11RasterizerState* oldRaster=nullptr;
        FLOAT factors[4];UINT mask,ref;context->OMGetBlendState(&oldBlend,factors,&mask);context->OMGetDepthStencilState(&oldDepth,&ref);context->RSGetState(&oldRaster);
        context->OMSetBlendState(nullptr,nullptr,0xffffffff);context->OMSetDepthStencilState(depth,0);context->RSSetState(raster);
        context->IASetInputLayout(layout);context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        UINT stride=sizeof(Plant),offset=0;context->IASetVertexBuffers(0,1,&instances,&stride,&offset);
        context->VSSetShader(vs,nullptr,0);context->PSSetShader(ps,nullptr,0);context->VSSetConstantBuffers(1,1,&constants);context->PSSetSamplers(0,1,&sampler);
        ViewData data;D3DXMatrixTranspose(&data.viewProjection,&vp);
        D3DXVECTOR3 right(camera.View._11,0,camera.View._31);D3DXVec3Normalize(&right,&right);
        data.right=D3DXVECTOR4(right.x,0,right.z,0);data.camera=D3DXVECTOR4(camera.Position.x,camera.Position.y,camera.Position.z,1);
        offset=0;
        for(unsigned type=0;type<3;++type)
        {
            if(!counts[type])continue;data.bounds=ImageBounds(type);context->UpdateSubresource(constants,0,nullptr,&data,0,0);
            context->PSSetShaderResources(0,1,&textures[type]);context->DrawInstanced(6,counts[type],0,offset);offset+=counts[type];
        }
        context->OMSetBlendState(oldBlend,factors,mask);context->OMSetDepthStencilState(oldDepth,ref);context->RSSetState(oldRaster);
        Free(oldBlend);Free(oldDepth);Free(oldRaster);
    }
    void Release()
    {
        for(auto& texture:textures)Free(texture);Free(vs);Free(ps);Free(layout);Free(instances);Free(constants);Free(sampler);Free(depth);Free(raster);
        for(auto& group:plants)group.clear();visible.clear();visibleCount=0;
    }
};
#endif
