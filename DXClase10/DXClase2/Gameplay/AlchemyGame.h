#ifndef GC_ALCHEMY_GAME_H
#define GC_ALCHEMY_GAME_H
#include "InteractionSystem.h"
#include <array>
#include <algorithm>
#include <sstream>

namespace Alchemy
{
enum class Ingredient { Mushroom, Flower, Crystal, Water, Herb, Root, Bottle, Potion };
enum class ItemState { Available, Carried, Placed, Processing, Processed, Consumed };
enum class StationState { Empty, Loaded, Working, Ready };
enum class BatchState { Empty, Loaded, Mixing, Mixed, Heated, Invalid };
enum class MatchState { Playing, Victory, Defeat };
inline const wchar_t* Name(Ingredient type)
{
    static const wchar_t* names[]={L"Hongo rojo",L"Flor azul",L"Cristal mágico",L"Agua",L"Hierba verde",L"Raíz luminosa",L"Frasco vacío",L"Poción"};
    return names[static_cast<int>(type)];
}
struct Requirement {Ingredient type;int quantity;bool ground;};
struct Recipe
{
    int id;std::wstring name;std::vector<Requirement> ingredients;
    float mixSeconds,heatMin,heatMax;bool ordered;int destination;float deadline;
};
inline std::vector<Recipe> DefaultRecipes()
{
    return {
        {0,L"Poción de Energía",{{Ingredient::Mushroom,1,false},{Ingredient::Flower,1,false},{Ingredient::Water,1,false}},3,0,0,false,0,180},
        {1,L"Poción de Velocidad",{{Ingredient::Herb,1,false},{Ingredient::Crystal,1,false},{Ingredient::Water,1,false}},3,3,6,false,1,180},
        {2,L"Poción de Vitalidad",{{Ingredient::Water,1,false},{Ingredient::Root,1,true},{Ingredient::Herb,1,false},{Ingredient::Mushroom,1,false}},4,5,8,true,2,210}
    };
}
struct Dose {Ingredient type;bool ground;};
struct Item
{
    int id;Ingredient type;ItemState state;Position position;float yaw=0;
    bool ground=false;int station=-1;int recipe=-1;unsigned order=0;bool correct=false;
    Item(int id,Ingredient type,Position p):id(id),type(type),state(ItemState::Available),position(p){}
};
struct Layout
{
    std::array<Position,6> sources;
    Position bottle;
    std::array<Position,4> stations;
    std::vector<Position> deliveries;
    Layout():sources{{{374,32.4f,399},{375,32.4f,399},{376,32.4f,399},{374,32.4f,407},{375,32.4f,407},{376,32.4f,407}}},
        bottle(384,32.4f,403),stations{{{390,32.5f,395},{401,32.3f,392},{394,31.5f,412},{390,32.5f,407}}},
        deliveries{{367,31.5f,419},{417,31.5f,424},{360,31.5f,445}}{}
};
struct Rules {int successGoal=5,failureLimit=3;float grindSeconds=3,betweenOrders=3;};

// Simulation-only module: no Windows, DirectX, mesh names or rendering calls.
class Game
{
    int nextItemId=1;
    void SetMessage(const std::wstring& text) {message=text;messageSeconds=5;}
    void StartOrder()
    {
        ++orderSerial;
        activeRecipe=static_cast<int>((orderSerial-1)%recipes.size());
        remaining=CurrentRecipe().deadline;showRecipe=true;
        items.clear();held=-1;mortarItem=-1;bottleItem=-1;
        mortar=StationState::Empty;grindElapsed=0;batch.clear();batchState=BatchState::Empty;
        mixElapsed=heatElapsed=0;heating=false;waiting=false;nextOrderSeconds=0;
        for(int type=0;type<6;++type)
        {
            int stock=1;
            for(const Recipe& r:recipes) for(const Requirement& req:r.ingredients)
                if(static_cast<int>(req.type)==type) stock=(std::max)(stock,req.quantity);
            for(int n=0;n<stock;++n)
            {Position p=layout.sources[type];p.z+=n*0.9f;items.emplace_back(nextItemId++,static_cast<Ingredient>(type),p);}
        }
        items.emplace_back(nextItemId++,Ingredient::Bottle,layout.bottle);
        SetMessage(L"Nuevo pedido: "+CurrentRecipe().name+L". R muestra la receta.");
    }
    void FinishOrder(bool success,const std::wstring& reason)
    {
        if(state!=MatchState::Playing || waiting) return;
        if(success) ++successes;else ++failures;
        heating=false;waiting=true;nextOrderSeconds=rules.betweenOrders;SetMessage(reason);
        if(successes>=rules.successGoal) state=MatchState::Victory;
        else if(failures>=rules.failureLimit) state=MatchState::Defeat;
    }
    void Take(int index)
    {
        if(held>=0) {SetMessage(L"Solo puedes llevar un objeto. Q para soltarlo.");return;}
        held=index;items[index].state=ItemState::Carried;items[index].station=-1;
        SetMessage(L"Llevas "+ItemName(items[index])+L".");
    }
    void MortarAction()
    {
        if(mortar==StationState::Empty)
        {
            if(held<0) {SetMessage(L"Trae un hongo, hierba o raíz para triturar.");return;}
            Item& item=items[held];
            if(item.ground || (item.type!=Ingredient::Mushroom && item.type!=Ingredient::Herb && item.type!=Ingredient::Root))
            {SetMessage(L"Este ingrediente no necesita o no admite triturado.");return;}
            mortarItem=held;item.state=ItemState::Placed;item.station=0;item.position=layout.stations[0];held=-1;mortar=StationState::Loaded;
            SetMessage(L"Ingrediente colocado. E en el mortero para triturar.");
        }
        else if(mortar==StationState::Loaded)
        {
            if(held>=0) {SetMessage(L"Deja libre tu mano antes de usar el mortero.");return;}
            mortar=StationState::Working;grindElapsed=0;items[mortarItem].state=ItemState::Processing;SetMessage(L"Triturando...");
        }
        else if(mortar==StationState::Ready)
        {
            if(held>=0) {SetMessage(L"Q para liberar la mano antes de recoger.");return;}
            const int item=mortarItem;mortarItem=-1;mortar=StationState::Empty;grindElapsed=0;Take(item);
        }
        else SetMessage(L"El mortero sigue triturando.");
    }
    void CauldronAction()
    {
        if(held>=0)
        {
            Item& item=items[held];
            if(static_cast<int>(item.type)>=6) {SetMessage(L"El caldero recibe ingredientes, no frascos ni pociones.");return;}
            if(batchState!=BatchState::Empty && batchState!=BatchState::Loaded) {SetMessage(L"La mezcla ya está cerrada.");return;}
            if(batch.size()>=4) {SetMessage(L"El caldero admite hasta cuatro unidades.");return;}
            batch.push_back({item.type,item.ground});item.state=ItemState::Consumed;held=-1;batchState=BatchState::Loaded;
            SetMessage(L"Ingrediente incorporado. E con la mano libre inicia la mezcla.");return;
        }
        if(batchState==BatchState::Loaded)
        {batchState=BatchState::Mixing;mixElapsed=0;SetMessage(L"Mezclando...");}
        else if(batchState==BatchState::Empty) SetMessage(L"Trae los ingredientes de la receta, uno a uno.");
        else SetMessage(L"Consulta el estado de la mezcla en el panel derecho.");
    }
    void HearthAction()
    {
        if(heating)
        {
            heating=false;
            if(heatElapsed>=CurrentRecipe().heatMin) {batchState=BatchState::Heated;SetMessage(L"Calor correcto. Lleva un frasco a embotellado.");}
            else {batchState=BatchState::Mixed;SetMessage(L"Calor insuficiente: puedes continuar calentando antes de sellar.");}
            return;
        }
        if(batchState!=BatchState::Mixed && batchState!=BatchState::Heated) {SetMessage(L"Primero prepara la mezcla en el caldero.");return;}
        if(CurrentRecipe().heatMax<=0) {SetMessage(L"Esta receta no necesita calor.");return;}
        heating=true;SetMessage(L"Fogón encendido. E de nuevo para detenerlo dentro del intervalo.");
    }
    void BottlerAction()
    {
        if(held>=0)
        {
            if(items[held].type!=Ingredient::Bottle || bottleItem>=0) {SetMessage(L"Coloca aquí un frasco vacío con la estación libre.");return;}
            bottleItem=held;items[held].state=ItemState::Placed;items[held].station=3;items[held].position=layout.stations[3];held=-1;
            SetMessage(L"Frasco colocado. E para embotellar la mezcla terminada.");return;
        }
        if(bottleItem<0) {SetMessage(L"Recoge el frasco vacío de la mesa y colócalo aquí.");return;}
        if(heating || (batchState!=BatchState::Mixed && batchState!=BatchState::Heated))
        {SetMessage(L"Falta terminar la mezcla y apagar el fogón.");return;}
        Item& potion=items[bottleItem];potion.type=Ingredient::Potion;potion.recipe=CurrentRecipe().id;potion.order=orderSerial;
        potion.correct=ValidateBatch() && mixElapsed>=CurrentRecipe().mixSeconds
            && (CurrentRecipe().heatMax<=0 || (heatElapsed>=CurrentRecipe().heatMin && heatElapsed<=CurrentRecipe().heatMax));
        const int finished=bottleItem;bottleItem=-1;batch.clear();batchState=BatchState::Empty;mixElapsed=heatElapsed=0;Take(finished);
        SetMessage(potion.correct?L"Poción correcta. Llévala al destino del pedido.":L"Poción con calor insuficiente. Su calidad es incorrecta.");
    }
public:
    Layout layout;Rules rules;
    std::vector<std::wstring> destinationNames{L"Vecina · señal dorada",L"Mensajero · señal azul",L"Guardabosques · señal verde"};
    std::vector<Recipe> recipes;
    std::vector<Item> items;
    std::vector<Dose> batch;
    MatchState state=MatchState::Playing;
    StationState mortar=StationState::Empty;
    BatchState batchState=BatchState::Empty;
    int held=-1,mortarItem=-1,bottleItem=-1,activeRecipe=0,successes=0,failures=0;
    unsigned orderSerial=0;
    float remaining=0,grindElapsed=0,mixElapsed=0,heatElapsed=0,nextOrderSeconds=0,messageSeconds=0;
    bool heating=false,waiting=false,showRecipe=true;
    std::wstring message;
    Game():recipes(DefaultRecipes()) {Restart();}
    const Recipe& CurrentRecipe() const {return recipes[activeRecipe];}
    const Item* Carried() const {return held>=0?&items[held]:nullptr;}
    static std::wstring ItemName(const Item& item)
    {return std::wstring(Name(item.type))+(item.ground?L" triturado":L"");}
    const wchar_t* DestinationName(int index) const
    {return index>=0 && index<static_cast<int>(destinationNames.size())?destinationNames[index].c_str():L"Destino sin nombre";}
    bool Active() const {return state==MatchState::Playing && !waiting;}
    void Restart()
    {
        if(recipes.empty()) recipes=DefaultRecipes();
        state=MatchState::Playing;successes=failures=0;orderSerial=0;nextItemId=1;showRecipe=true;StartOrder();
    }
    bool ValidateBatch() const
    {
        const Recipe& recipe=CurrentRecipe();
        size_t expected=0;for(const Requirement& r:recipe.ingredients) expected+=r.quantity;
        if(batch.size()!=expected) return false;
        if(recipe.ordered)
        {
            size_t index=0;
            for(const Requirement& r:recipe.ingredients) for(int n=0;n<r.quantity;++n,++index)
                if(batch[index].type!=r.type || (r.ground && !batch[index].ground)) return false;
        }
        else for(const Requirement& r:recipe.ingredients)
        {
            int count=0;
            for(const Dose& dose:batch) if(dose.type==r.type && (!r.ground || dose.ground)) ++count;
            if(count!=r.quantity) return false;
        }
        return true;
    }
    void Tick(float seconds)
    {
        if(!std::isfinite(seconds) || seconds<=0 || state!=MatchState::Playing) return;
        messageSeconds=(std::max)(0.0f,messageSeconds-seconds);
        if(waiting)
        {nextOrderSeconds-=seconds;if(nextOrderSeconds<=0) StartOrder();return;}
        remaining=(std::max)(0.0f,remaining-seconds);
        if(remaining<=0) {heating=false;FinishOrder(false,L"Se agotó el tiempo del pedido.");return;}
        if(mortar==StationState::Working)
        {
            grindElapsed=(std::min)(rules.grindSeconds,grindElapsed+seconds);
            if(grindElapsed>=rules.grindSeconds)
            {mortar=StationState::Ready;items[mortarItem].ground=true;items[mortarItem].state=ItemState::Processed;SetMessage(L"Triturado listo. E para recuperar el ingrediente.");}
        }
        if(batchState==BatchState::Mixing)
        {
            mixElapsed=(std::min)(CurrentRecipe().mixSeconds,mixElapsed+seconds);
            if(mixElapsed>=CurrentRecipe().mixSeconds)
            {
                if(!ValidateBatch()) {batchState=BatchState::Invalid;FinishOrder(false,L"Mezcla inválida: revisa ingredientes, cantidades, triturado y orden.");return;}
                batchState=BatchState::Mixed;SetMessage(CurrentRecipe().heatMax>0?L"Mezcla lista. Controla el tiempo en el fogón.":L"Mezcla lista. Puedes embotellar sin calentar.");
            }
        }
        if(heating)
        {
            heatElapsed+=seconds;
            if(heatElapsed>CurrentRecipe().heatMax)
            {batchState=BatchState::Invalid;heating=false;FinishOrder(false,L"La mezcla se quemó: pedido fallido.");}
        }
    }
    void Interact(const Target& target)
    {
        if(!Active()) return;
        if(target.kind==TargetKind::Item)
        {
            if(target.index<0 || target.index>=static_cast<int>(items.size())) return;
            Item& item=items[target.index];
            if(target.id!=1000+item.id || item.station>=0 || (item.state!=ItemState::Available && item.state!=ItemState::Processed)) return;
            Take(target.index);
        }
        else if(target.kind==TargetKind::Mortar) MortarAction();
        else if(target.kind==TargetKind::Cauldron) CauldronAction();
        else if(target.kind==TargetKind::Hearth) HearthAction();
        else if(target.kind==TargetKind::Bottler) BottlerAction();
        else if(target.kind==TargetKind::Delivery)
        {
            if(held<0 || items[held].type!=Ingredient::Potion) {SetMessage(L"Lleva una poción embotellada al destino.");return;}
            Item& item=items[held];
            const bool correct=target.index==CurrentRecipe().destination && item.correct && item.recipe==CurrentRecipe().id && item.order==orderSerial;
            item.state=ItemState::Consumed;held=-1;FinishOrder(correct,correct?L"¡Entrega correcta!":L"Entrega incorrecta: revisa calidad, receta y destino.");
        }
    }
    void Drop(Position position)
    {
        if(held<0 || !Active()) return;
        items[held].position=position;items[held].station=-1;items[held].state=ItemState::Available;held=-1;SetMessage(L"Objeto soltado. E para recuperarlo.");
    }
    void UpdateCarry(Position position,float yaw)
    {if(held>=0) {items[held].position=position;items[held].yaw=yaw;}}
    std::vector<Target> Candidates() const
    {
        std::vector<Target> targets;if(!Active()) return targets;
        if(held<0) for(size_t i=0;i<items.size();++i)
        {
            const Item& item=items[i];
            if(item.station<0 && (item.state==ItemState::Available || item.state==ItemState::Processed))
                targets.emplace_back(TargetKind::Item,static_cast<int>(i),1000+item.id,item.position,L"E - Recoger "+ItemName(item));
        }
        const wchar_t* mortarLabel=mortar==StationState::Ready?L"E - Recuperar triturado":mortar==StationState::Loaded?L"E - Triturar":mortar==StationState::Working?L"Mortero: triturando...":L"E - Colocar en mortero";
        const wchar_t* cauldronLabel=held>=0?L"E - Añadir al caldero":batchState==BatchState::Loaded?L"E - Mezclar":L"E - Revisar caldero";
        targets.emplace_back(TargetKind::Mortar,0,10,layout.stations[0],mortarLabel);
        targets.emplace_back(TargetKind::Cauldron,1,11,layout.stations[1],cauldronLabel);
        targets.emplace_back(TargetKind::Hearth,2,12,layout.stations[2],heating?L"E - Apagar fogón":L"E - Encender fogón");
        targets.emplace_back(TargetKind::Bottler,3,13,layout.stations[3],held>=0?L"E - Colocar frasco":L"E - Embotellar");
        for(int i=0;i<static_cast<int>(layout.deliveries.size());++i) targets.emplace_back(TargetKind::Delivery,i,20+i,layout.deliveries[i],Carried() && Carried()->type==Ingredient::Potion?L"E - Entregar poción":std::wstring(L"Entrega: ")+DestinationName(i));
        return targets;
    }
};
}
#endif
