#ifndef GC_WATER_STREAM_H
#define GC_WATER_STREAM_H
#include "MeshPrimitives.h"
#include "Camara.h"
#include <vector>

// A shallow visual ribbon beside the village. It follows the existing terrain;
// it does not register a collider, excavate the map or change movement rules.
class WaterStream
{
    struct Vertex {D3DXVECTOR3 position;D3DXVECTOR2 uv;D3DXVECTOR3 normal;};
    struct Point {D3DXVECTOR3 position;float halfWidth;};std::vector<Point> path;
    struct ViewData {D3DXMATRIX viewProjection;D3DXVECTOR4 camera,time;};
    ID3D11Buffer *vertices=nullptr,*indices=nullptr,*constants=nullptr,*lighting=nullptr;
    ID3D11VertexShader* vs=nullptr;ID3D11PixelShader* ps=nullptr;ID3D11InputLayout* layout=nullptr;
    ID3D11ShaderResourceView* noise=nullptr;ID3D11SamplerState* sampler=nullptr;
    ID3D11BlendState* blend=nullptr;ID3D11DepthStencilState* depth=nullptr;ID3D11RasterizerState* raster=nullptr;
    UINT indexCount=0;
    template<typename T> static void Free(T*& p){if(p){p->Release();p=nullptr;}}
    bool Texture(ID3D11Device* device,ID3D11DeviceContext* context)
    {
        const unsigned size=128;std::vector<unsigned char> data(size*size*4);
        for(unsigned y=0;y<size;++y)for(unsigned x=0;x<size;++x)
        {
            const float u=x*D3DX_PI*2/size,v=y*D3DX_PI*2/size;
            const float a=.5f+.19f*sinf(u*3+v*2)+.14f*cosf(u*7-v*5)+.10f*sinf(u*11+v*3);
            const float b=.5f+.22f*sinf(u*5-v*3)+.17f*cosf(u*2+v*7);
            const unsigned i=(y*size+x)*4;data[i]=static_cast<unsigned char>(a*255);data[i+1]=static_cast<unsigned char>(b*255);data[i+2]=180;data[i+3]=255;
        }
        D3D11_TEXTURE2D_DESC desc={};desc.Width=desc.Height=size;desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;
        ID3D11Texture2D* texture=nullptr;if(FAILED(device->CreateTexture2D(&desc,nullptr,&texture)))return false;
        context->UpdateSubresource(texture,0,nullptr,data.data(),size*4,0);const HRESULT hr=device->CreateShaderResourceView(texture,nullptr,&noise);texture->Release();
        if(FAILED(hr))return false;context->GenerateMips(noise);return true;
    }
public:
    float waveAmplitude=.035f;
    WaterStream()=default;WaterStream(const WaterStream&)=delete;WaterStream& operator=(const WaterStream&)=delete;
    ~WaterStream(){Release();}
    bool Contains(float x,float z,float margin=0) const
    {
        if(x<328-margin || x>347+margin || z<346-margin || z>449+margin)return false;
        for(size_t i=1;i<path.size();++i)
        {
            const auto& a=path[i-1];const auto& b=path[i];const float dx=b.position.x-a.position.x,dz=b.position.z-a.position.z;
            const float t=(std::max)(0.0f,(std::min)(1.0f,((x-a.position.x)*dx+(z-a.position.z)*dz)/(dx*dx+dz*dz)));
            const float px=x-a.position.x-t*dx,pz=z-a.position.z-t*dz,width=a.halfWidth+(b.halfWidth-a.halfWidth)*t+margin+.35f;
            if(px*px+pz*pz<width*width)return true;
        }
        return false;
    }
    bool Init(ID3D11Device* device,ID3D11DeviceContext* context,MPrimitives& terrain)
    {
        Release();const unsigned rows=128,columns=10;std::vector<Vertex> mesh;std::vector<unsigned> triangles;
        for(unsigned row=0;row<=rows;++row)
        {
            const float t=static_cast<float>(row)/rows,x=337+4*sinf(t*D3DX_PI*2)-t,z=350+95*t,half=2.3f+.4f*sinf(t*D3DX_PI);
            const float dx=8*D3DX_PI*cosf(t*D3DX_PI*2)-1,dz=95;D3DXVECTOR2 across(dz,-dx);D3DXVec2Normalize(&across,&across);
            path.push_back({D3DXVECTOR3(x,terrain.getTerrainCollision({x,z}),z),half});
            for(unsigned col=0;col<=columns;++col)
            {
                const float u=static_cast<float>(col)/columns;D3DXVECTOR3 p,n;
                if(!terrain.SampleTerrainSurface({x+across.x*(u*2-1)*half,z+across.y*(u*2-1)*half},p,n))return false;
                p.y+=.14f;mesh.push_back({p,{u,t*12},n});
                if(row<rows && col<columns)
                {const unsigned a=row*(columns+1)+col,b=a+1,c=a+columns+1,d=c+1;for(unsigned i:{a,c,b,b,c,d})triangles.push_back(i);}
            }
        }
        indexCount=static_cast<UINT>(triangles.size());D3D11_BUFFER_DESC desc={};desc.Usage=D3D11_USAGE_IMMUTABLE;desc.ByteWidth=static_cast<UINT>(mesh.size()*sizeof(Vertex));desc.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data={};data.pSysMem=mesh.data();if(FAILED(device->CreateBuffer(&desc,&data,&vertices))){Release();return false;}
        desc.ByteWidth=indexCount*sizeof(unsigned);desc.BindFlags=D3D11_BIND_INDEX_BUFFER;data.pSysMem=triangles.data();if(FAILED(device->CreateBuffer(&desc,&data,&indices))){Release();return false;}
        ID3DBlob* v=nullptr;ID3DBlob* p=nullptr;bool ok=CompileSceneShader(L"waterStream.hlsl","VS","vs_4_0",&v) && CompileSceneShader(L"waterStream.hlsl","PS","ps_4_0",&p);
        const D3D11_INPUT_ELEMENT_DESC elements[]={{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0}};
        if(ok)ok=SUCCEEDED(device->CreateVertexShader(v->GetBufferPointer(),v->GetBufferSize(),nullptr,&vs))
            && SUCCEEDED(device->CreatePixelShader(p->GetBufferPointer(),p->GetBufferSize(),nullptr,&ps))
            && SUCCEEDED(device->CreateInputLayout(elements,3,v->GetBufferPointer(),v->GetBufferSize(),&layout));
        Free(v);Free(p);if(!ok || !Texture(device,context)){Release();return false;}
        desc={};desc.Usage=D3D11_USAGE_DEFAULT;desc.ByteWidth=sizeof(ViewData);desc.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        if(FAILED(device->CreateBuffer(&desc,nullptr,&constants))){Release();return false;}
        desc.ByteWidth=sizeof(Light);if(FAILED(device->CreateBuffer(&desc,nullptr,&lighting))){Release();return false;}
        D3D11_SAMPLER_DESC sample={};sample.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sample.AddressU=sample.AddressV=sample.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;sample.MaxLOD=D3D11_FLOAT32_MAX;
        D3D11_BLEND_DESC alpha={};auto& target=alpha.RenderTarget[0];target.BlendEnable=TRUE;target.SrcBlend=D3D11_BLEND_SRC_ALPHA;target.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;target.BlendOp=D3D11_BLEND_OP_ADD;
        target.SrcBlendAlpha=D3D11_BLEND_ONE;target.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;target.BlendOpAlpha=D3D11_BLEND_OP_ADD;target.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
        D3D11_DEPTH_STENCIL_DESC ds={};ds.DepthEnable=TRUE;ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;ds.DepthFunc=D3D11_COMPARISON_LESS_EQUAL;
        D3D11_RASTERIZER_DESC rs={};rs.FillMode=D3D11_FILL_SOLID;rs.CullMode=D3D11_CULL_NONE;rs.DepthClipEnable=TRUE;
        if(FAILED(device->CreateSamplerState(&sample,&sampler)) || FAILED(device->CreateBlendState(&alpha,&blend))
            || FAILED(device->CreateDepthStencilState(&ds,&depth)) || FAILED(device->CreateRasterizerState(&rs,&raster))){Release();return false;}return true;
    }
    void Draw(ID3D11DeviceContext* context,const Camara& camera,const Light& light,float seconds)
    {
        if(!vertices)return;ID3D11BlendState* oldBlend=nullptr;ID3D11DepthStencilState* oldDepth=nullptr;ID3D11RasterizerState* oldRaster=nullptr;
        FLOAT factors[4];UINT mask,ref;context->OMGetBlendState(&oldBlend,factors,&mask);context->OMGetDepthStencilState(&oldDepth,&ref);context->RSGetState(&oldRaster);
        context->OMSetBlendState(blend,nullptr,0xffffffff);context->OMSetDepthStencilState(depth,0);context->RSSetState(raster);
        ViewData data;const D3DXMATRIX vp=camera.View*camera.Projection;D3DXMatrixTranspose(&data.viewProjection,&vp);
        data.camera=D3DXVECTOR4(camera.Position.x,camera.Position.y,camera.Position.z,1);data.time=D3DXVECTOR4(seconds,waveAmplitude,1.0f/128,0);
        context->UpdateSubresource(constants,0,nullptr,&data,0,0);context->VSSetConstantBuffers(1,1,&constants);context->PSSetConstantBuffers(1,1,&constants);
        context->UpdateSubresource(lighting,0,nullptr,&light,0,0);context->PSSetConstantBuffers(0,1,&lighting);
        UINT stride=sizeof(Vertex),offset=0;context->IASetVertexBuffers(0,1,&vertices,&stride,&offset);context->IASetIndexBuffer(indices,DXGI_FORMAT_R32_UINT,0);
        context->IASetInputLayout(layout);context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vs,nullptr,0);context->PSSetShader(ps,nullptr,0);context->PSSetShaderResources(0,1,&noise);context->PSSetSamplers(0,1,&sampler);context->DrawIndexed(indexCount,0,0);
        context->OMSetBlendState(oldBlend,factors,mask);context->OMSetDepthStencilState(oldDepth,ref);context->RSSetState(oldRaster);Free(oldBlend);Free(oldDepth);Free(oldRaster);
    }
    void Release()
    {Free(vertices);Free(indices);Free(constants);Free(lighting);Free(vs);Free(ps);Free(layout);Free(noise);Free(sampler);Free(blend);Free(depth);Free(raster);path.clear();indexCount=0;}
};
#endif
