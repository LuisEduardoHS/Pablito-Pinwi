#ifndef GC_HUD_IMAGE_H
#define GC_HUD_IMAGE_H
#include <algorithm>
#include <cmath>
#include <d3dx11.h>
#include <vector>
#include <cstring>

namespace Ui
{
// Decode once, then composite an image into an RGBA HUD without extra GPU draws.
// The supplied files remain unchanged; only their visible bounds are fitted.
class HudImage
{
    struct Icon {unsigned width=0,height=0;int left=0,top=0,right=0,bottom=0;std::vector<unsigned char> rgba;};
    Icon icon;
public:
    bool Load(ID3D11Device* device,ID3D11DeviceContext* context,const wchar_t* path,unsigned maxSide=256)
    {
        D3DX11_IMAGE_INFO source={};if(FAILED(D3DX11GetImageInfoFromFile(path,nullptr,&source,nullptr)))return false;
        if(!source.Width || !source.Height)return false;
        const float scale=(std::min)(1.0f,static_cast<float>(maxSide)/(std::max)(source.Width,source.Height));
        D3DX11_IMAGE_LOAD_INFO info;info.Width=(std::max)(1u,static_cast<unsigned>(source.Width*scale+.5f));
        info.Height=(std::max)(1u,static_cast<unsigned>(source.Height*scale+.5f));info.MipLevels=1;
        info.Usage=D3D11_USAGE_STAGING;info.BindFlags=info.MiscFlags=0;info.CpuAccessFlags=D3D11_CPU_ACCESS_READ;info.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        ID3D11Resource* resource=nullptr;if(FAILED(D3DX11CreateTextureFromFile(device,path,&info,nullptr,&resource,nullptr)))return false;
        ID3D11Texture2D* texture=nullptr;const HRESULT query=resource->QueryInterface(__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&texture));resource->Release();
        if(FAILED(query))return false;
        D3D11_TEXTURE2D_DESC desc={};texture->GetDesc(&desc);D3D11_MAPPED_SUBRESOURCE mapped={};
        const HRESULT hr=context->Map(texture,0,D3D11_MAP_READ,0,&mapped);
        if(SUCCEEDED(hr))
        {
            icon.width=desc.Width;icon.height=desc.Height;icon.rgba.resize(static_cast<size_t>(desc.Width)*desc.Height*4);
            icon.left=desc.Width;icon.top=desc.Height;icon.right=icon.bottom=0;
            for(unsigned y=0;y<desc.Height;++y)
            {
                const auto* row=static_cast<const unsigned char*>(mapped.pData)+y*mapped.RowPitch;
                std::memcpy(icon.rgba.data()+y*desc.Width*4,row,desc.Width*4);
                for(unsigned x=0;x<desc.Width;++x)if(row[x*4+3]>8)
                {icon.left=(std::min)(icon.left,static_cast<int>(x));icon.top=(std::min)(icon.top,static_cast<int>(y));icon.right=(std::max)(icon.right,static_cast<int>(x)+1);icon.bottom=(std::max)(icon.bottom,static_cast<int>(y)+1);}
            }
            context->Unmap(texture,0);
        }
        texture->Release();return SUCCEEDED(hr) && icon.right>icon.left && icon.bottom>icon.top;
    }
    void Draw(unsigned char* pixels,int width,int height,int x,int y,int boxWidth,int boxHeight) const
    {
        if(!pixels || boxWidth<=0 || boxHeight<=0 || icon.rgba.empty())return;
        const int sourceW=icon.right-icon.left,sourceH=icon.bottom-icon.top;
        const float scale=(std::min)(static_cast<float>(boxWidth)/sourceW,static_cast<float>(boxHeight)/sourceH);
        const int w=(std::max)(1,static_cast<int>(sourceW*scale+.5f)),h=(std::max)(1,static_cast<int>(sourceH*scale+.5f));
        x+=(boxWidth-w)/2;y+=(boxHeight-h)/2;
        for(int row=0;row<h;++row)for(int col=0;col<w;++col)
        {
            if(x+col<0 || x+col>=width || y+row<0 || y+row>=height)continue;
            const float sx=icon.left+(col+.5f)*sourceW/w-.5f,sy=icon.top+(row+.5f)*sourceH/h-.5f;
            const int ix=static_cast<int>(floorf(sx)),iy=static_cast<int>(floorf(sy));const float fx=sx-ix,fy=sy-iy;
            float alpha=0,color[3]={};
            for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx)
            {
                const int px=(std::max)(0,(std::min)(static_cast<int>(icon.width)-1,ix+dx)),py=(std::max)(0,(std::min)(static_cast<int>(icon.height)-1,iy+dy));
                const unsigned char* sample=icon.rgba.data()+(py*icon.width+px)*4;
                const float weight=(dx?fx:1-fx)*(dy?fy:1-fy)*sample[3]/255.0f;alpha+=weight;
                for(unsigned c=0;c<3;++c)color[c]+=sample[c]*weight;
            }
            if(alpha<=0)continue;unsigned char* dest=pixels+((y+row)*width+x+col)*4;
            const float behind=dest[3]/255.0f*(1-alpha),out=alpha+behind;
            for(unsigned c=0;c<3;++c)dest[c]=static_cast<unsigned char>((std::min)(255.0f,(color[c]+dest[c]*behind)/out+.5f));
            dest[3]=static_cast<unsigned char>((std::min)(255.0f,out*255+.5f));
        }
    }
    void Clear(){icon=Icon();}
};
}
#endif
