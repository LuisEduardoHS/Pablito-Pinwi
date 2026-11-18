// CPU regression of visual eligibility/time. Run with the x86 DirectX SDK.
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include "../Constants.h"
#include "../Camara.h"
#include "../Light.h"
#include "../SceneEffects.h"
#pragma comment(lib,"d3dx10.lib")
static unsigned checks=0;
static void Check(bool ok,const char* text){++checks;if(!ok){fprintf(stderr,"FAIL: %s\n",text);exit(1);}}
int main()
{
    SceneEffects effects;Alchemy::Target item(Alchemy::TargetKind::Item,0,1001,{0,0,0},L"E - Recoger",3.5f);
    float previous=0;
    for(int step=0;step<=35;++step)
    {
        effects.UpdateHighlights({item},{3.5f-step*.1f,0,0},item.id);const float strength=effects.HighlightStrength(item.id);
        Check(strength>=previous-.00001f && strength>=0 && strength<=1.2f,"proximity fade is bounded and monotonic");previous=strength;
    }
    const float selected=effects.HighlightStrength(item.id);effects.UpdateHighlights({item},{0,0,0},-1);
    Check(selected>effects.HighlightStrength(item.id),"selected target has stronger emphasis");
    effects.UpdateHighlights({item},{3.6f,0,0},item.id);Check(effects.HighlightStrength(item.id)==0,"no highlight out of range");
    effects.UpdateHighlights({item},{0,4,0},item.id);Check(effects.HighlightStrength(item.id)==0,"existing vertical reach respected");
    item.visible=false;effects.UpdateHighlights({item},{0,0,0},item.id);Check(effects.HighlightStrength(item.id)==0,"blocked line of sight suppresses highlight");
    item.visible=true;item.range=0;effects.UpdateHighlights({item},{0,0,0},item.id);Check(effects.HighlightStrength(item.id)==0,"zero range does not divide by zero");
    item.range=3.5f;effects.UpdateHighlights({item},{0,0,0},item.id);effects.Reset();Check(effects.HighlightStrength(item.id)==0 && effects.Time()==0,"restart clears visual state");
    SceneEffects slow,fast;for(int i=0;i<30;++i)slow.Advance(1.0f/15);for(int i=0;i<288;++i)fast.Advance(1.0f/144);
    Check(fabsf(slow.Time()-fast.Time())<.0001f,"effect clock independent of FPS");
    const float time=slow.Time();slow.Advance(-1);slow.Advance(std::numeric_limits<float>::quiet_NaN());Check(slow.Time()==time,"invalid elapsed times ignored");
    const auto a=SceneEffects::PotionColor(0),b=SceneEffects::PotionColor(1),c=SceneEffects::PotionColor(2);
    Check(a.x>b.x && b.z>a.z && c.y>c.x,"recipes retain distinguishable emission colors");
    printf("PASS: %u scene effect checks; proximity, selection, range/visibility, reset, clock and recipe colors.\n",checks);
}
