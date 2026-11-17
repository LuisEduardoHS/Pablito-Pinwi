#ifndef GC_GAME_SCREENS_H
#define GC_GAME_SCREENS_H
#include "../InteractionPrompt.h"
#include "HudImage.h"
#include <string>
#include <algorithm>
#include <cstring>

// Screen artwork and button input, separate from the in-game HUD.
// Button styling/layout can change without changing the game actions.
class GameScreens
{
public:
    enum class Screen { Title, Gameplay, Victory, Defeat };
    enum class Action { None, Start, Restart, Exit };
private:
    Screen screen=Screen::Title;int selected=0,hovered=-1,pressed=-1;
    int width=0,height=0;bool dirty=true;
    Ui::HudImage victoryBanner,defeatBanner;
    ID3D11Texture2D* texture=nullptr;ID3D11ShaderResourceView* view=nullptr;
    HDC dc=nullptr;HBITMAP bitmap=nullptr;HGDIOBJ oldBitmap=nullptr;
    HFONT titleFont=nullptr,font=nullptr;unsigned char* pixels=nullptr;
    template<typename T> static void Free(T*& value){if(value){value->Release();value=nullptr;}}
    RECT Layout(int index) const
    {
        const int w=(std::min)(260,width-48),x=(width-w)/2,y=height/2+12+index*58;
        return {x,y,x+w,y+44};
    }
    int Count() const {return screen==Screen::Title?1:screen==Screen::Gameplay?0:2;}
    int Hit(int x,int y,int clientWidth,int clientHeight) const
    {
        if(clientWidth<=0 || clientHeight<=0)return -1;
        const POINT p={MulDiv(x,width,clientWidth),MulDiv(y,height,clientHeight)};
        for(int i=0;i<Count();++i){RECT r=Layout(i);if(PtInRect(&r,p))return i;}return -1;
    }
    Action ButtonAction(int index) const
    {return index<0 || index>=Count()?Action::None:screen==Screen::Title?Action::Start:index==0?Action::Restart:Action::Exit;}
    void Text(RECT rect,const std::wstring& text,HFONT face,COLORREF color)
    {
        HGDIOBJ old=SelectObject(dc,face);SetTextColor(dc,color);
        DrawTextW(dc,text.c_str(),static_cast<int>(text.size()),&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);SelectObject(dc,old);
    }
    void Paint()
    {
        RECT full={0,0,width,height};HBRUSH background=CreateSolidBrush(RGB(12,17,25));FillRect(dc,&full,background);DeleteObject(background);
        const int center=height/2;
        if(screen==Screen::Title)Text({16,center-126,width-16,center-78},L"PABLITO PINWI",titleFont,RGB(232,232,227));
        for(int i=0;i<Count();++i)
        {
            RECT rect=Layout(i);const bool active=(hovered>=0?hovered:selected)==i;
            HBRUSH fill=CreateSolidBrush(pressed==i?RGB(40,52,66):RGB(22,29,40));FillRect(dc,&rect,fill);DeleteObject(fill);
            HBRUSH border=CreateSolidBrush(active?RGB(218,225,230):RGB(77,92,108));FrameRect(dc,&rect,border);DeleteObject(border);
            Text(rect,screen==Screen::Title?L"Iniciar":i==0?L"Reiniciar":L"Salir",font,RGB(232,232,227));
        }
        GdiFlush();
        for(size_t i=0;i<static_cast<size_t>(width)*height;++i)
        {unsigned char* p=pixels+i*4;const unsigned char b=p[0];p[0]=p[2];p[2]=b;p[3]=255;}
        if(screen==Screen::Victory || screen==Screen::Defeat)
        {
            const int w=(std::min)(560,width-64),h=(std::min)(160,height/3);
            const Ui::HudImage& banner=screen==Screen::Victory?victoryBanner:defeatBanner;
            banner.Draw(pixels,width,height,(width-w)/2,center-96-h/2,w,h);
        }
    }
    bool Resize(ID3D11Device* device,int w,int h)
    {
        ReleaseSurface();width=w;height=h;dc=CreateCompatibleDC(nullptr);if(!dc)return false;
        BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        void* data=nullptr;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&data,nullptr,0);pixels=static_cast<unsigned char*>(data);
        if(!bitmap || !pixels){ReleaseSurface();return false;}oldBitmap=SelectObject(dc,bitmap);SetBkMode(dc,TRANSPARENT);
        titleFont=CreateFontW(-30,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,L"Courier New");
        font=CreateFontW(-17,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,L"Courier New");
        D3D11_TEXTURE2D_DESC desc={};desc.Width=w;desc.Height=h;desc.MipLevels=desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        if(!titleFont || !font || FAILED(device->CreateTexture2D(&desc,nullptr,&texture)) || FAILED(device->CreateShaderResourceView(texture,nullptr,&view)))
        {ReleaseSurface();return false;}dirty=true;return true;
    }
public:
    GameScreens()=default;GameScreens(const GameScreens&)=delete;GameScreens& operator=(const GameScreens&)=delete;
    ~GameScreens(){Release();}
    bool Init(ID3D11Device* device,int w,int h)
    {
        ID3D11DeviceContext* context=nullptr;device->GetImmediateContext(&context);
        const bool loaded=victoryBanner.Load(device,context,L"Texturas/Pantallas/ganaste.png",1024)
            && defeatBanner.Load(device,context,L"Texturas/Pantallas/has-perdido.png",1024);
        context->Release();if(!loaded){Release();return false;}return Resize(device,w,h);
    }
    void SetScreen(Screen next)
    {if(screen==next)return;screen=next;selected=0;hovered=pressed=-1;dirty=true;}
    Action Key(WPARAM key)
    {
        if(!Count())return Action::None;
        if(key==VK_UP || key==VK_LEFT || key==VK_DOWN || key==VK_RIGHT || key==VK_TAB)
        {const int direction=key==VK_UP || key==VK_LEFT?-1:1;selected=(selected+Count()+direction)%Count();hovered=-1;dirty=true;}
        return key==VK_RETURN || key==VK_SPACE?ButtonAction(selected):Action::None;
    }
    void PointerMove(int x,int y,int w,int h)
    {const int next=Hit(x,y,w,h);if(next!=hovered){hovered=next;dirty=true;}}
    void PointerDown(int x,int y,int w,int h)
    {pressed=Hit(x,y,w,h);hovered=pressed;dirty=true;}
    Action PointerUp(int x,int y,int w,int h)
    {const int hit=Hit(x,y,w,h);const Action action=hit==pressed?ButtonAction(hit):Action::None;CancelClick();return action;}
    void CancelClick(){pressed=-1;dirty=true;}
    void Draw(ID3D11Device* device,ID3D11DeviceContext* context,InteractionPrompt& renderer)
    {
        D3D11_VIEWPORT viewport={};UINT count=1;context->RSGetViewports(&count,&viewport);if(!count)return;
        if(width!=static_cast<int>(viewport.Width) || height!=static_cast<int>(viewport.Height))
            if(!Resize(device,static_cast<int>(viewport.Width),static_cast<int>(viewport.Height)))return;
        if(dirty && pixels){Paint();context->UpdateSubresource(texture,0,nullptr,pixels,width*4,0);dirty=false;}
        renderer.DrawScreen(view,0,0,static_cast<float>(width),static_cast<float>(height));
    }
    void Release(){victoryBanner.Clear();defeatBanner.Clear();ReleaseSurface();}
private:
    void ReleaseSurface()
    {
        Free(view);Free(texture);if(dc && oldBitmap)SelectObject(dc,oldBitmap);
        if(bitmap)DeleteObject(bitmap);if(titleFont)DeleteObject(titleFont);if(font)DeleteObject(font);if(dc)DeleteDC(dc);
        dc=nullptr;bitmap=nullptr;oldBitmap=nullptr;titleFont=font=nullptr;pixels=nullptr;width=height=0;dirty=true;
    }
};
#endif
