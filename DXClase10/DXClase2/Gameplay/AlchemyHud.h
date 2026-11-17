#ifndef GC_ALCHEMY_HUD_H
#define GC_ALCHEMY_HUD_H
#include "AlchemyGame.h"
#include "../InteractionPrompt.h"
#include <iomanip>

namespace Alchemy
{
// Transparent classic text. Only prose is typed out; readings stay immediate.
class Hud
{
    enum : COLORREF { Ink=RGB(225,223,214), Muted=RGB(166,184,193), Warning=RGB(220,158,143), Shadow=RGB(8,11,14) };
    ID3D11Texture2D* texture=nullptr;ID3D11ShaderResourceView* view=nullptr;
    HDC dc=nullptr;HBITMAP bitmap=nullptr;HGDIOBJ previousBitmap=nullptr;
    HFONT font=nullptr,titleFont=nullptr,smallFont=nullptr;
    unsigned char* pixels=nullptr;int width=0,height=0;
    float refresh=0,noticeAge=0,recipeAge=0,lastNoticeSeconds=0;
    bool recipeWasOpen=false;int lastOrder=-1;std::wstring lastNotice,lastRecipe;
    void Text(int x,int y,int w,int h,const std::wstring& text,COLORREF color=Ink,HFONT face=nullptr,
        UINT flags=DT_LEFT|DT_TOP|DT_WORDBREAK|DT_NOPREFIX)
    {
        HGDIOBJ old=SelectObject(dc,face?face:font);SetTextColor(dc,Shadow);
        const int offsets[][2]={{-1,0},{1,0},{0,-1},{0,1}};
        for(const auto& offset:offsets)
        {RECT r={x+offset[0],y+offset[1],x+w+offset[0],y+h+offset[1]};DrawTextW(dc,text.c_str(),static_cast<int>(text.size()),&r,flags);}
        SetTextColor(dc,color);RECT r={x,y,x+w,y+h};DrawTextW(dc,text.c_str(),static_cast<int>(text.size()),&r,flags);SelectObject(dc,old);
    }
    void Center(int x,int y,int w,int h,const std::wstring& text,COLORREF color=Ink,HFONT face=nullptr)
    {Text(x,y,w,h,text,color,face,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);}
    static std::wstring Number(float value,int digits=1)
    {std::wostringstream s;s<<std::fixed<<std::setprecision(digits)<<value;return s.str();}
    static std::wstring Clock(float value)
    {
        const int seconds=(std::max)(0,static_cast<int>(ceilf(value)));
        std::wostringstream s;s<<seconds/60<<L":"<<std::setw(2)<<std::setfill(L'0')<<seconds%60;return s.str();
    }
    static std::wstring ShortDestination(const Game& game)
    {std::wstring name=game.DestinationName(game.CurrentRecipe().destination);return name.substr(0,name.find(L" ·"));}
    static std::wstring Reveal(const std::wstring& text,float age,float rate)
    {
        const size_t count=static_cast<size_t>((std::max)(0.0f,age)*rate);
        return count>=text.size()?text:text.substr(0,count)+L"_";
    }
    static std::wstring Progress(float value)
    {
        const int filled=static_cast<int>((std::max)(0.0f,(std::min)(1.0f,value))*10);
        return L"["+std::wstring(filled,L'=')+std::wstring(10-filled,L'.')+L"]";
    }
    void DrawOrder(const Game& game)
    {
        Text(18,16,350,20,L"PEDIDO "+std::to_wstring(game.orderSerial)+L" / "+game.CurrentRecipe().name,Ink,titleFont);
        Text(18,40,350,18,L"Para "+ShortDestination(game)+L"   "+Clock(game.remaining),game.remaining<20?Warning:Muted);
        Text(18,61,350,18,L"Entregas "+std::to_wstring(game.successes)+L"/"+std::to_wstring(game.rules.successGoal)
            +L"   Fallos "+std::to_wstring(game.failures)+L"/"+std::to_wstring(game.rules.failureLimit),Muted,smallFont);
    }
    void DrawRecipe(const Game& game)
    {
        const Recipe& recipe=game.CurrentRecipe();Text(18,101,320,18,recipe.ordered?L"RECETA / EN ESTE ORDEN  [R]":L"RECETA  [R]",Muted);
        std::wstring body;
        for(size_t i=0;i<recipe.ingredients.size();++i)
        {
            const Requirement& req=recipe.ingredients[i];
            body+=(recipe.ordered?std::to_wstring(i+1)+L". ":L"  ")+std::to_wstring(req.quantity)+L" x "+Name(req.type)
                +(req.ground?L" (triturada)":L"")+L"\n";
        }
        body+=L"\nMezclar "+Number(recipe.mixSeconds,0)+L" s\n";
        body+=recipe.heatMax>0?L"Calentar "+Number(recipe.heatMin,0)+L" a "+Number(recipe.heatMax,0)+L" s":L"Sin fuego";
        body+=L"\nEmbotellar y entregar.";Text(18,126,326,height-270,Reveal(body,recipeAge,80));
    }
    void DrawCarry(const Game& game)
    {
        const int x=width-320;Text(x,18,302,18,L"MANO / "+(game.Carried()?Game::ItemName(*game.Carried()):L"libre"),Ink,nullptr,DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
        if(game.Carried())
        {
            std::wstring hint=L"[Q] Soltar";
            if(game.Carried()->type==Ingredient::Potion)hint=std::wstring(game.Carried()->correct?L"Poción correcta":L"Poción incorrecta")+L"   "+hint;
            Text(x,40,302,18,hint,Muted,smallFont,DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
        }
    }
    void DrawStations(const Game& game,const Target* target)
    {
        const int x=width-302;int y=88;
        auto row=[&](const std::wstring& text,COLORREF color=Muted){Text(x,y,284,36,text,color,smallFont);y+=40;};
        if(game.mortar!=StationState::Empty || (target && target->kind==TargetKind::Mortar))
        {
            const wchar_t* states[]={L"vacío",L"[E] triturar",L"triturando",L"listo"};
            std::wstring text=L"MORTERO / "+std::wstring(states[static_cast<int>(game.mortar)]);
            if(game.mortar==StationState::Working)text+=L"\n"+Progress(game.grindElapsed/(std::max)(0.01f,game.rules.grindSeconds));row(text);
        }
        if(game.batchState!=BatchState::Empty || (target && target->kind==TargetKind::Cauldron))
        {
            const wchar_t* states[]={L"vacío",L"ingredientes",L"mezclando",L"mezcla lista",L"caliente",L"error"};
            std::wstring text=L"CALDERO / "+std::wstring(states[static_cast<int>(game.batchState)])+L" ("+std::to_wstring(game.batch.size())+L"/4)";
            if(game.batchState==BatchState::Mixing)text+=L"\n"+Progress(game.mixElapsed/(std::max)(0.01f,game.CurrentRecipe().mixSeconds));row(text);
        }
        if(game.heating || game.heatElapsed>0 || (target && target->kind==TargetKind::Hearth))
        {
            const Recipe& r=game.CurrentRecipe();const std::wstring status=game.heating?Number(game.heatElapsed)+L" s":L"apagado";
            row(L"FOGÓN / "+status+L"\nCalor necesario: "+Number(r.heatMin,0)+L" a "+Number(r.heatMax,0)+L" s",
                r.heatMax>0 && game.heatElapsed>r.heatMax*0.85f?Warning:Muted);
        }
        if(game.bottleItem>=0 || (target && target->kind==TargetKind::Bottler))row(game.bottleItem>=0?L"EMBOTELLADO / frasco listo":L"EMBOTELLADO / sin frasco");
    }
    void DrawAction(const Game& game,const std::wstring& action)
    {
        std::wstring text=action;if(text.find(L"E - ")==0)text=L"[E] "+text.substr(4);
        if(game.waiting)text=L"Siguiente pedido en "+Number((std::max)(0.0f,game.nextOrderSeconds),0)+L" s";
        if(text.empty())text=game.showRecipe?L"[R] Ocultar receta":L"[R] Ver receta";
        Center(18,height-56,width-36,24,text);Center(18,height-28,width-36,18,L"R receta   Q soltar   C cámara",Muted,smallFont);
    }
    bool Resize(ID3D11Device* device,int w,int h)
    {
        Release();width=w;height=h;dc=CreateCompatibleDC(nullptr);BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        void* data=nullptr;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&data,nullptr,0);pixels=static_cast<unsigned char*>(data);
        if(!dc || !bitmap || !pixels)return false;previousBitmap=SelectObject(dc,bitmap);SetBkMode(dc,TRANSPARENT);
        auto makeFont=[](int size){return CreateFontW(-size,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,L"Courier New");};
        font=makeFont(14);titleFont=makeFont(16);smallFont=makeFont(12);
        D3D11_TEXTURE2D_DESC desc={};desc.Width=w;desc.Height=h;desc.MipLevels=desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        return SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&texture)) && SUCCEEDED(device->CreateShaderResourceView(texture,nullptr,&view));
    }
public:
    Hud()=default;Hud(const Hud&)=delete;Hud& operator=(const Hud&)=delete;~Hud(){Release();}
    bool Init(ID3D11Device* device,int w,int h){return Resize(device,w,h);}
    void Draw(ID3D11Device* device,ID3D11DeviceContext* context,InteractionPrompt& renderer,const Game& game,const std::wstring& action,bool focused,float seconds,const Target* target=nullptr)
    {
        if(game.state!=MatchState::Playing)return;
        D3D11_VIEWPORT viewport={};UINT count=1;context->RSGetViewports(&count,&viewport);if(!count)return;
        if(static_cast<int>(viewport.Width)!=width || static_cast<int>(viewport.Height)!=height)if(!Resize(device,static_cast<int>(viewport.Width),static_cast<int>(viewport.Height)))return;
        if(lastNotice!=game.message || game.messageSeconds>lastNoticeSeconds+0.1f){lastNotice=game.message;noticeAge=0;refresh=0;}lastNoticeSeconds=game.messageSeconds;
        if(game.showRecipe && (!recipeWasOpen || lastOrder!=game.orderSerial || lastRecipe!=game.CurrentRecipe().name)){recipeAge=0;refresh=0;}
        recipeWasOpen=game.showRecipe;lastOrder=game.orderSerial;lastRecipe=game.CurrentRecipe().name;
        const float elapsed=focused?(std::max)(0.0f,(std::min)(seconds,0.1f)):0.0f;
        noticeAge+=elapsed;if(game.showRecipe)recipeAge+=elapsed;refresh-=seconds;
        if(refresh<=0 && pixels)
        {
            refresh=0.04f;std::memset(pixels,0,static_cast<size_t>(width)*height*4);DrawOrder(game);
            if(game.state==MatchState::Playing)
            {
                DrawCarry(game);if(game.showRecipe)DrawRecipe(game);DrawStations(game,target);DrawAction(game,action);
                if(game.messageSeconds>0 && game.message.find(L"Llevas ")!=0)Text(18,height-119,(std::min)(520,width-36),48,L"> "+Reveal(game.message,noticeAge,48),Muted);
                if(!focused)Center(18,16,width-36,18,L"PAUSA",Muted,smallFont);
            }
            GdiFlush();
            for(size_t i=0;i<static_cast<size_t>(width)*height;++i)
            {
                unsigned char* p=pixels+i*4;const COLORREF color=RGB(p[2],p[1],p[0]);const unsigned char b=p[0];p[0]=p[2];p[2]=b;
                p[3]=color==0?0:color==Shadow?220:255;
            }
            context->UpdateSubresource(texture,0,nullptr,pixels,width*4,0);
        }
        renderer.DrawScreen(view,0,0,static_cast<float>(width),static_cast<float>(height));
    }
    void Invalidate(){refresh=0;}
    void Release()
    {
        if(view){view->Release();view=nullptr;}if(texture){texture->Release();texture=nullptr;}
        if(dc && previousBitmap)SelectObject(dc,previousBitmap);
        if(bitmap)DeleteObject(bitmap);if(font)DeleteObject(font);if(titleFont)DeleteObject(titleFont);if(smallFont)DeleteObject(smallFont);if(dc)DeleteDC(dc);
        dc=nullptr;bitmap=nullptr;font=titleFont=smallFont=nullptr;previousBitmap=nullptr;pixels=nullptr;width=height=0;refresh=0;
    }
};
}
#endif
