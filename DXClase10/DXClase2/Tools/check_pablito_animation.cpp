// Standalone x86 CPU check; June 2010 DirectX SDK and d3dx10.lib required.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include "../Gameplay/PablitoAnimator.h"
#pragma comment(lib,"d3dx10.lib")
using Motion=PablitoAnimator::Motion;
static unsigned checks=0;
static void Check(bool condition,const char* message)
{++checks;if(!condition){fprintf(stderr,"FAIL: %s\n",message);exit(1);}}
static D3DXVECTOR3 Point(const PablitoAnimator& animator,unsigned bone,D3DXVECTOR3 point)
{D3DXVECTOR3 out;D3DXVec3TransformCoord(&out,&point,&animator.Matrices()[bone]);return out;}
static float Distance(D3DXVECTOR3 a,D3DXVECTOR3 b){D3DXVECTOR3 d=a-b;return D3DXVec3Length(&d);}
static void Step(PablitoAnimator& animator,const PablitoAnimator::Input& input,int frames=60,float dt=1.0f/60)
{
    for(int i=0;i<frames;++i)
    {
        animator.Update(dt,input);
        for(unsigned bone=0;bone<PablitoRigData::BoneCount;++bone)
            for(unsigned element=0;element<16;++element)Check(std::isfinite(reinterpret_cast<const float*>(&animator.Matrices()[bone])[element]),"all pose matrices finite");
    }
}
int main()
{
    unsigned counts[9]={};for(auto bone:PablitoRigData::TriangleBones){Check(bone<9,"valid bone assignment");++counts[bone];}
    Check(sizeof(PablitoRigData::TriangleBones)*3==PablitoRigData::VertexCount,"complete triangle rig");
    for(auto count:counts)Check(count>0,"every rig part has vertices");
    PablitoAnimator animator;PablitoAnimator::Input input;Step(animator,input,20);Check(animator.Current()==Motion::Idle,"idle pose");
    const D3DXVECTOR3 foot(-.03f,.38f,-.42f);Check(Distance(Point(animator,4,foot),foot)<.001f,"idle feet planted while breathing");
    const auto idleTip=Point(animator,2,{-.12f,1.63f,-1.40f});input.carrying=true;Step(animator,input,50);
    Check(animator.Current()==Motion::Carry,"carry overlay active");Check(Distance(idleTip,Point(animator,2,{-.12f,1.63f,-1.40f}))>.5f,"flippers move into carrying pose");
    Check(Distance(animator.ToolHand(),animator.Grip())<.75f,"carrying flipper reaches the held object");
    input.speed=8;Step(animator,input,20);Check(animator.Current()==Motion::Walk,"walking while carrying");
    auto walkFoot=Point(animator,4,{.5f,.12f,-.42f});Step(animator,input,12);
    Check(Distance(walkFoot,Point(animator,4,{.5f,.12f,-.42f}))>.05f,"walking moves feet");
    input.speed=16;Step(animator,input,15);Check(animator.Current()==Motion::Run,"run follows actual speed");
    input.speed=0;input.grounded=false;input.verticalSpeed=4;Step(animator,input,4);Check(animator.Current()==Motion::Jump,"takeoff pose");
    input.verticalSpeed=-3;Step(animator,input,4);Check(animator.Current()==Motion::Fall,"falling pose");
    input.grounded=true;Step(animator,input,1);Check(animator.Current()==Motion::Land,"landing reaction");Step(animator,input,30);
    Check(animator.Current()==Motion::Carry,"landing blends back to carrying");
    for(Motion action:{Motion::Pickup,Motion::Place,Motion::Drop,Motion::Grind,Motion::Stir,Motion::Heat,Motion::Bottle,Motion::Deliver,Motion::Read,Motion::Nope})
    {
        animator.Play(action);Step(animator,input,3);Check(animator.Current()==action,"interaction clip starts");
        Step(animator,input,80);Check(animator.Current()==Motion::Carry,"interaction returns to base pose");
    }
    input.carrying=false;input.work=Motion::Grind;Step(animator,input,100);Check(animator.Current()==Motion::Grind,"grinding loops while station active");
    input.work=Motion::Stir;Step(animator,input,100);Check(animator.Current()==Motion::Stir,"stirring loops while station active");
    input.work=Motion::Heat;Step(animator,input,100);Check(animator.Current()==Motion::Heat,"hearth pose loops");
    input.work=Motion::Idle;input.riding=true;animator.Play(Motion::Mount,{0,-1.4f,-2});
    Check(Distance(animator.TransitionOffset(),{0,-1.4f,-2})<.001f,"mount starts at old feet");Step(animator,input,60);
    Check(animator.Current()==Motion::Ride && Distance(animator.TransitionOffset(),{0,0,0})<.001f,"mount finishes on cloud");
    input.steering=.8f;Step(animator,input,40);auto leaning=Point(animator,0,{0,2.5f,0});input.steering=-.8f;Step(animator,input,40);
    Check(Distance(leaning,Point(animator,0,{0,2.5f,0}))>.1f,"rider balances through turns");
    input.riding=false;animator.Play(Motion::Dismount,{1,1,0});Step(animator,input,50);Check(animator.Current()==Motion::Idle,"dismount returns to idle");
    input.victory=true;animator.Play(Motion::Pickup);Step(animator,input,40);Check(animator.Current()==Motion::Victory,"victory overrides pending action");
    auto happy=Point(animator,2,{-.12f,1.63f,-1.40f});input.victory=false;input.defeat=true;Step(animator,input,40);
    Check(fabsf(happy.z)>1.1f && happy.y>1.8f,"celebration flipper stays visible outside the large head");
    Check(animator.Current()==Motion::Defeat && Distance(happy,Point(animator,2,{-.12f,1.63f,-1.40f}))>1,"distinct defeat pose");
    const float clock=animator.Time();animator.Update(std::numeric_limits<float>::quiet_NaN(),input);animator.Update(-1,input);
    Check(animator.Time()==clock,"invalid elapsed times ignored");
    PablitoAnimator slow,fast;input={};input.speed=8;Step(slow,input,15*4,1.0f/15);Step(fast,input,144*4,1.0f/144);
    Check(Distance(Point(slow,4,{.5f,.12f,-.42f}),Point(fast,4,{.5f,.12f,-.42f}))<.13f,"walking timing independent of FPS");
    animator.Reset();Check(animator.Current()==Motion::Idle && animator.Time()==0,"restart clears previous clips");
    printf("PASS: %u checks; rig, locomotion/carry, air/landing, interactions/work, cloud, results, reset, finite poses and FPS.\n",checks);
}
