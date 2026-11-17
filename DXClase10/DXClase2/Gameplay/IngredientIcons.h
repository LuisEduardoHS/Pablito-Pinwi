#ifndef GC_INGREDIENT_ICONS_H
#define GC_INGREDIENT_ICONS_H
#include "AlchemyGame.h"
#include "HudImage.h"
namespace Alchemy
{
class IngredientIcons
{
    std::array<Ui::HudImage,6> icons;
public:
    bool Init(ID3D11Device* device)
    {
        Clear();ID3D11DeviceContext* context=nullptr;device->GetImmediateContext(&context);
        const wchar_t* paths[]={L"Texturas/Ingredientes/hongo-rojo.png",L"Texturas/Ingredientes/flor-azul.png",
            L"Texturas/Ingredientes/cristal-morado.png",L"Texturas/Ingredientes/agua.png",L"Texturas/Ingredientes/hierba-verde.png",L"Texturas/Ingredientes/raiz-luminosa.png"};
        bool ok=true;for(unsigned i=0;i<icons.size() && ok;++i)ok=icons[i].Load(device,context,paths[i]);context->Release();if(!ok)Clear();return ok;
    }
    static bool Has(Ingredient type){return static_cast<unsigned>(type)<6;}
    void Draw(unsigned char* pixels,int width,int height,Ingredient type,int x,int y,int box) const
    {if(Has(type))icons[static_cast<unsigned>(type)].Draw(pixels,width,height,x,y,box,box);}
    void Clear(){for(auto& icon:icons)icon.Clear();}
};
}
#endif