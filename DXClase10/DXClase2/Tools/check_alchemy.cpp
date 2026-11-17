// Standalone gameplay tests; no DirectX or Windows required.
// cl /EHsc /std:c++14 /utf-8 Tools/check_alchemy.cpp /Fe:check_alchemy.exe
#include "../Gameplay/AlchemyGame.h"
#include <cstdio>
#include <cstdlib>
using namespace Alchemy;
static int checks=0;
static void Check(bool ok,const char* message) {++checks;if(!ok){fprintf(stderr,"FAIL: %s\n",message);exit(1);}}
static void Use(Game& g,TargetKind kind,int index=0) {g.Interact(Target(kind,index,10+index));}
static void Pick(Game& g,Ingredient type)
{
    for(const Target& t:g.Candidates()) if(t.kind==TargetKind::Item && g.items[t.index].type==type) {g.Interact(t);return;}
    Check(false,"requested item is available");
}
static void Prepare(Game& g,bool heat=true)
{
    const Recipe recipe=g.CurrentRecipe();
    for(const Requirement& req:recipe.ingredients) for(int n=0;n<req.quantity;++n)
    {
        Pick(g,req.type);
        if(req.ground) {Use(g,TargetKind::Mortar);Use(g,TargetKind::Mortar);g.Tick(g.rules.grindSeconds);Use(g,TargetKind::Mortar);Check(g.Carried() && g.Carried()->ground,"mortar recovery processed");}
        Use(g,TargetKind::Cauldron);
    }
    Check(g.ValidateBatch(),"recipe accepts actual doses");Use(g,TargetKind::Cauldron);g.Tick(recipe.mixSeconds);
    Check(g.batchState==BatchState::Mixed && g.Active(),"mix finishes without failure");
    if(heat && recipe.heatMax>0) {Use(g,TargetKind::Hearth);g.Tick(recipe.heatMin+0.1f);Use(g,TargetKind::Hearth);Check(g.batchState==BatchState::Heated && !g.heating,"controlled heat");}
    Pick(g,Ingredient::Bottle);Use(g,TargetKind::Bottler);Check(g.held<0,"bottle placed first");Use(g,TargetKind::Bottler);
    Check(g.Carried() && g.Carried()->type==Ingredient::Potion,"transportable potion created");
}
int main()
{
    { // Phases 1-2: selection, one carried item, stable identity and drop.
        std::vector<Target> targets={Target(TargetKind::Item,0,1,{0,0,2}),Target(TargetKind::Item,1,2,{0,0,-2})};
        Check(SelectTarget({0,0,0},{0,0,1},targets)==0,"prefer facing at equal distance");
        targets[0].visible=false;Check(SelectTarget({0,0,0},{0,0,1},targets)==1,"occlusion excludes candidate");
        Check(SelectTarget({20,0,0},{0,0,1},targets)<0,"configurable range");
        Game g;Pick(g,Ingredient::Mushroom);int first=g.held;int count=0;
        for(const Item& i:g.items) if(i.state==ItemState::Carried)++count;
        Check(count==1 && g.Carried()->type==Ingredient::Mushroom,"one carried item");
        Target stale(TargetKind::Item,1,1000+g.items[1].id);g.Interact(stale);Check(g.held==first,"cannot carry second object");
        g.UpdateCarry({4,5,6},1.2f);Check(g.items[first].position.x==4 && g.items[first].yaw==1.2f,"carry pose follows player");
        g.Drop({7,8,9});Check(g.held<0 && g.items[first].state==ItemState::Available && g.items[first].position.z==9,"drop preserves identity");
    }
    { // Phase 3: first complete recipe before expanding the pipeline.
        Game g;Prepare(g);Check(g.Carried()->correct,"energy potion correct");
        unsigned serial=g.Carried()->order;int identity=g.Carried()->id;g.Drop({1,2,3});Pick(g,Ingredient::Potion);
        Check(g.Carried()->correct && g.Carried()->order==serial && g.Carried()->id==identity,"potion drop/recovery preserves quality and order metadata");
        Use(g,TargetKind::Delivery,g.CurrentRecipe().destination);Check(g.successes==1 && g.held<0 && g.waiting,"correct delivery consumes potion");
        Use(g,TargetKind::Delivery,0);Check(g.successes==1,"delivery cannot count twice");
    }
    { // Phases 4-6: all recipes, processing, victory and restart.
        Game g;
        for(int order=0;order<5;++order)
        {
            Prepare(g);Check(g.Carried()->correct && g.Carried()->order==g.orderSerial,"recipe/order metadata");
            Use(g,TargetKind::Delivery,g.CurrentRecipe().destination);
            if(order<4)g.Tick(g.rules.betweenOrders+0.1f);
        }
        Check(g.state==MatchState::Victory && g.successes==5,"five deliveries win");
        g.Tick(1000);Check(g.successes==5 && g.failures==0,"terminal state freezes");
        g.Restart();Check(g.Active() && g.successes==0 && g.failures==0 && g.held<0,"restart clears all state");
    }
    {
        Game g;Pick(g,Ingredient::Mushroom);Use(g,TargetKind::Cauldron);Pick(g,Ingredient::Crystal);Use(g,TargetKind::Cauldron);Pick(g,Ingredient::Water);Use(g,TargetKind::Cauldron);
        Check(!g.ValidateBatch(),"same quantity with wrong types rejected");Use(g,TargetKind::Cauldron);g.Tick(3);
        Check(g.failures==1 && g.waiting,"invalid mix fails once");g.Tick(0.1f);Use(g,TargetKind::Cauldron);Check(g.failures==1,"invalid mix not double counted");
    }
    {
        Game g;g.Tick(181);g.Tick(4); // Velocity next.
        Prepare(g,false);Check(!g.Carried()->correct,"unheated velocity seals incorrect potion");
        Use(g,TargetKind::Delivery,g.CurrentRecipe().destination);Check(g.failures==2,"bad quality rejected at delivery");g.Tick(4);
        const Recipe recipe=g.CurrentRecipe();
        for(const auto& req:recipe.ingredients) {Pick(g,req.type);Use(g,TargetKind::Cauldron);}
        Check(!g.ValidateBatch(),"required grinding enforced");
        g.batch.clear();for(auto it=recipe.ingredients.rbegin();it!=recipe.ingredients.rend();++it)g.batch.push_back({it->type,true});
        Check(!g.ValidateBatch(),"required sequence enforced");
    }
    {
        Game g;g.Tick(181);g.Tick(4);const Recipe recipe=g.CurrentRecipe();
        for(const auto& req:recipe.ingredients) {Pick(g,req.type);Use(g,TargetKind::Cauldron);}
        Use(g,TargetKind::Cauldron);g.Tick(recipe.mixSeconds);Use(g,TargetKind::Hearth);g.Tick(recipe.heatMax+0.1f);
        Check(g.failures==2 && g.batchState==BatchState::Invalid && !g.heating,"overheat invalidates once");
        g.Tick(4);g.Tick(1000);Check(g.state==MatchState::Defeat && g.failures==3,"third failed order loses");
        g.Tick(1000);Check(g.failures==3,"defeat not counted again");
    }
    {
        Game g;Prepare(g);Use(g,TargetKind::Delivery,2);Check(g.failures==1 && g.successes==0,"wrong destination fails");
        g.Restart();Pick(g,Ingredient::Bottle);Use(g,TargetKind::Bottler);Use(g,TargetKind::Bottler);Check(!g.Carried() && g.bottleItem>=0,"no potion from empty cauldron");
        g.Tick(-1);g.Tick(NAN);Check(g.remaining==180,"invalid delta ignored");
    }
    {
        Game g;g.recipes[0].ingredients[2].quantity=2;g.Restart();Prepare(g);
        Check(g.Carried()->correct,"recipe quantities configurable; stock and validation follow data");
        g.Restart();g.batch={{Ingredient::Mushroom,false},{Ingredient::Flower,false},{Ingredient::Water,false}};
        Check(!g.ValidateBatch(),"missing unit rejected despite correct ingredient types");
        g.batch.push_back({Ingredient::Water,false});Check(g.ValidateBatch(),"required duplicate quantity accepted");
        g.batch.push_back({Ingredient::Water,false});Check(!g.ValidateBatch(),"extra quantity rejected");
    }
    {
        Game g;g.Tick(181);g.Tick(4);auto recipe=g.CurrentRecipe();
        for(auto req:recipe.ingredients){Pick(g,req.type);Use(g,TargetKind::Cauldron);}
        Use(g,TargetKind::Cauldron);g.Tick(recipe.mixSeconds);Use(g,TargetKind::Hearth);g.Tick(1);Use(g,TargetKind::Hearth);
        Check(!g.heating && g.batchState==BatchState::Mixed && !g.waiting,"early heat stop can be corrected before sealing");
        Use(g,TargetKind::Hearth);g.Tick(recipe.heatMin-1+0.1f);Use(g,TargetKind::Hearth);
        Check(g.batchState==BatchState::Heated && g.heatElapsed>=recipe.heatMin,"resuming heat accumulates time");
        Use(g,TargetKind::Hearth);Pick(g,Ingredient::Bottle);Use(g,TargetKind::Hearth);
        Check(!g.heating && g.Carried() && g.Carried()->type==Ingredient::Bottle,"can stop fire while carrying a bottle");
    }
    printf("PASS: %d gameplay checks (interaction, carry, recipes, all stations, heat, orders, deliveries, win/loss, restart).\n",checks);
}
