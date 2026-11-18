// Standalone x86 CPU regression; DirectX June 2010 SDK + d3dx10.lib.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "../Gameplay/SecretEmoteCode.h"
#include "../Gameplay/PablitoAnimator.h"
#pragma comment(lib,"d3dx10.lib")
static unsigned checks=0;
static void Check(bool ok,const char* why){++checks;if(!ok){fprintf(stderr,"FAIL: %s\n",why);exit(1);}}
static bool Code(SecretEmoteCode& code)
{
    for(int i=0;i<6;++i)Check(!code.Press('6'),"six alone never activates");
    for(int i=0;i<6;++i)Check(!code.Press('7'),"incomplete seven suffix never activates");
    return code.Press('7');
}
static D3DXVECTOR3 Tip(const PablitoAnimator& a,unsigned bone,float side)
{D3DXVECTOR3 p(-.12f,1.63f,side*1.40f),out;D3DXVec3TransformCoord(&out,&p,&a.Matrices()[bone]);return out;}
int main()
{
    SecretEmoteCode code;Check(Code(code),"exact sequence activates");Check(Code(code),"can repeat full sequence");
    code.Press('6');for(int i=0;i<20;++i)Check(!code.Press('6',true),"held key cannot advance");
    for(int i=0;i<7;++i)Check(!code.Press('7'),"one physical six is insufficient");
    for(int i=0;i<6;++i)code.Press('6');code.Press('R');for(int i=0;i<7;++i)Check(!code.Press('7'),"other key breaks code");
    code.Reset();for(int i=0;i<7;++i)code.Press('6');for(int i=0;i<7;++i)Check(!code.Press('7'),"extra six breaks exact prefix");
    code.Reset();for(int i=0;i<6;++i)code.Press('6');code.Reset();for(int i=0;i<7;++i)Check(!code.Press('7'),"focus/menu reset clears prefix");
    for(int i=0;i<6;++i)code.Press(VK_NUMPAD6);for(int i=0;i<6;++i)code.Press(VK_NUMPAD7);Check(code.Press(VK_NUMPAD7),"numpad supported");
    PablitoAnimator a;PablitoAnimator::Input input;a.Play(PablitoAnimator::Motion::SixSeven);
    for(int i=0;i<12;++i)a.Update(1.0f/60,input);
    const float left=Tip(a,2,-1).y,right=Tip(a,3,1).y;
    for(int i=0;i<24;++i)a.Update(1.0f/60,input);
    Check(left>Tip(a,2,-1).y+.25f && right<Tip(a,3,1).y-.25f,"flippers alternate height in opposite phase");
    Check(a.Current()==PablitoAnimator::Motion::SixSeven,"distinct emote motion");
    for(int i=0;i<330;++i)a.Update(1.0f/60,input);Check(a.Current()==PablitoAnimator::Motion::Idle,"returns to idle after clip");
    a.Play(PablitoAnimator::Motion::SixSeven);input.speed=8;a.Update(.016f,input);Check(a.Current()==PablitoAnimator::Motion::Walk,"movement interrupts emote");
    a.Play(PablitoAnimator::Motion::SixSeven);input.speed=0;input.grounded=false;a.Update(.016f,input);Check(a.Current()==PablitoAnimator::Motion::Fall,"jump interrupts emote");
    input={};a.Play(PablitoAnimator::Motion::SixSeven);a.Play(PablitoAnimator::Motion::Pickup);a.Update(.016f,input);Check(a.Current()==PablitoAnimator::Motion::Pickup,"interaction replaces emote");
    a.Play(PablitoAnimator::Motion::SixSeven);a.Reset();Check(!a.IsPlaying(PablitoAnimator::Motion::SixSeven),"restart clears emote");
    printf("PASS: %u checks; exact secret sequence, repeat/reset, numpad, alternating flippers, duration and interruption.\n",checks);
}
