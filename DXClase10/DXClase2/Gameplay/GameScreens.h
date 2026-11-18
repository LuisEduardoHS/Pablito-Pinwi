#ifndef GC_GAME_SCREENS_H
#define GC_GAME_SCREENS_H
#include "../InteractionPrompt.h"
#include "HudImage.h"
#include <algorithm>
#include <vector>

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
    Ui::HudImage titleBanner,playButton,restartButton,exitButton,victoryBanner,defeatBanner;
    ID3D11Texture2D* texture=nullptr;ID3D11ShaderResourceView* view=nullptr;
    std::vector<unsigned char> pixels;
    template<typename T> static void Free(T*& value){if(value){value->Release();value=nullptr;}}
    float Scale() const
    {return (std::min)(1.5f,(std::min)(width/800.0f,height/600.0f));}
    RECT Layout(int index) const
    {
        const float s=Scale();const int w=static_cast<int>(280*s),h=static_cast<int>(82*s);
        const int x=(width-w)/2,y=height/2+static_cast<int>((screen==Screen::Title?110:8+index*106)*s);
        return {x,y,x+w,y+h};
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
    // Small translucent rounded layers provide focus/press feedback without
    // recoloring the supplied artwork or introducing extra labels.
    void Rounded(RECT rect,int radius,unsigned char r,unsigned char g,unsigned char b,float alpha)
    {
        for(int y=(std::max)(0L,rect.top);y<(std::min)(static_cast<LONG>(height),rect.bottom);++y)
        for(int x=(std::max)(0L,rect.left);x<(std::min)(static_cast<LONG>(width),rect.right);++x)
        {
            const int cx=(std::max)(rect.left+radius,(std::min)(rect.right-radius-1,static_cast<LONG>(x)));
            const int cy=(std::max)(rect.top+radius,(std::min)(rect.bottom-radius-1,static_cast<LONG>(y)));
            const int dx=x-cx,dy=y-cy;if(dx*dx+dy*dy>radius*radius)continue;
            auto* p=pixels.data()+(static_cast<size_t>(y)*width+x)*4;
            p[0]=static_cast<unsigned char>(p[0]*(1-alpha)+r*alpha);
            p[1]=static_cast<unsigned char>(p[1]*(1-alpha)+g*alpha);
            p[2]=static_cast<unsigned char>(p[2]*(1-alpha)+b*alpha);
        }
    }
    void Paint()
    {
        const float s=Scale();const int center=height/2;
        // Calm blue backdrop, cached until selection or viewport changes.
        for(int y=0;y<height;++y)for(int x=0;x<width;++x)
        {
            const float v=static_cast<float>(y)/height,dx=(x-width*.5f)/width,dy=(y-height*.38f)/height;
            const float glow=(std::max)(0.0f,1-(dx*dx*2+dy*dy)*3);
            auto* p=pixels.data()+(static_cast<size_t>(y)*width+x)*4;
            p[0]=static_cast<unsigned char>(22+22*v+12*glow);
            p[1]=static_cast<unsigned char>(34+31*v+20*glow);
            p[2]=static_cast<unsigned char>(62+39*v+27*glow);p[3]=255;
        }
        const int bannerW=static_cast<int>(600*s),bannerH=static_cast<int>((screen==Screen::Title?235:180)*s);
        const int bannerY=center-static_cast<int>((screen==Screen::Title?180:210)*s);
        const Ui::HudImage& banner=screen==Screen::Title?titleBanner:screen==Screen::Victory?victoryBanner:defeatBanner;
        banner.Draw(pixels.data(),width,height,(width-bannerW)/2,bannerY,bannerW,bannerH);
        for(int i=0;i<Count();++i)
        {
            RECT rect=Layout(i);const bool active=(hovered>=0?hovered:selected)==i;
            RECT shadow=rect;OffsetRect(&shadow,0,static_cast<int>(5*s));Rounded(shadow,static_cast<int>(14*s),7,15,33,.32f);
            if(active){RECT halo=rect;InflateRect(&halo,static_cast<int>(5*s),static_cast<int>(5*s));Rounded(halo,static_cast<int>(18*s),181,221,255,pressed==i?.55f:.30f);}
            if(pressed==i){InflateRect(&rect,-static_cast<int>(3*s),-static_cast<int>(2*s));OffsetRect(&rect,0,static_cast<int>(2*s));}
            const Ui::HudImage& button=screen==Screen::Title?playButton:i==0?restartButton:exitButton;
            button.Draw(pixels.data(),width,height,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
        }
    }
    bool Resize(ID3D11Device* device,int w,int h)
    {
        ReleaseSurface();if(w<=0 || h<=0)return false;width=w;height=h;pixels.resize(static_cast<size_t>(w)*h*4);
        D3D11_TEXTURE2D_DESC desc={};desc.Width=w;desc.Height=h;desc.MipLevels=desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        if(FAILED(device->CreateTexture2D(&desc,nullptr,&texture)) || FAILED(device->CreateShaderResourceView(texture,nullptr,&view)))
        {ReleaseSurface();return false;}dirty=true;return true;
    }
public:
    GameScreens()=default;GameScreens(const GameScreens&)=delete;GameScreens& operator=(const GameScreens&)=delete;
    ~GameScreens(){Release();}
    bool Init(ID3D11Device* device,int w,int h)
    {
        ID3D11DeviceContext* context=nullptr;device->GetImmediateContext(&context);
        const bool loaded=titleBanner.Load(device,context,L"Texturas/Pantallas/letrero-pablitopinwi.png",1536)
            && playButton.Load(device,context,L"Texturas/Pantallas/boton-jugar.png",1536)
            && restartButton.Load(device,context,L"Texturas/Pantallas/boton-reiniciar.png",1024)
            && exitButton.Load(device,context,L"Texturas/Pantallas/boton-salir.png",1024)
            && victoryBanner.Load(device,context,L"Texturas/Pantallas/ganaste.png",1024)
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
        if(dirty && !pixels.empty()){Paint();context->UpdateSubresource(texture,0,nullptr,pixels.data(),width*4,0);dirty=false;}
        renderer.DrawScreen(view,0,0,static_cast<float>(width),static_cast<float>(height));
    }
    void Release(){titleBanner.Clear();playButton.Clear();restartButton.Clear();exitButton.Clear();victoryBanner.Clear();defeatBanner.Clear();ReleaseSurface();}
private:
    void ReleaseSurface()
    {
        Free(view);Free(texture);pixels.clear();width=height=0;dirty=true;
    }
};
#endif
