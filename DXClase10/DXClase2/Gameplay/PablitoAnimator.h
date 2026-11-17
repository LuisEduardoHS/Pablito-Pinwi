#ifndef GC_PABLITO_ANIMATOR_H
#define GC_PABLITO_ANIMATOR_H
#include <d3dx10.h>
#include <cmath>
#include <algorithm>
#include "PablitoRigData.h"

// Procedural part rig for the original bird. Angles blend before matrices are
// built, preserving rigid head/feet/flippers rather than stretching triangles.
class PablitoAnimator
{
public:
    enum class Motion { Idle,Walk,Run,Carry,Jump,Fall,Land,Mount,Ride,Dismount,
        Pickup,Place,Drop,Grind,Stir,Heat,Bottle,Deliver,Read,Nope,Victory,Defeat };
    struct Input
    {
        float speed=0,verticalSpeed=0,lookYaw=0,lookPitch=0,steering=0;
        bool grounded=true,riding=false,carrying=false,victory=false,defeat=false;
        Motion work=Motion::Idle;
    };
private:
    struct Pose
    {
        D3DXVECTOR3 body={0,0,0},head={0,0,0},bodyOffset={0,0,0};
        D3DXVECTOR3 leftWing={-1.0f,0,0},rightWing={1.0f,0,0};
        D3DXVECTOR3 leftReach={0,0,0},rightReach={0,0,0};
        D3DXVECTOR3 leftFoot={0,0,0},rightFoot={0,0,0},leftStep={0,0,0},rightStep={0,0,0};
        float scaleY=1,eyeOpen=1,mouth=0;
    } pose;
    Motion current=Motion::Idle,event=Motion::Idle;
    float time=0,phase=0,eventAge=0,eventLength=0;bool wasGrounded=true;
    D3DXVECTOR3 transitionFrom={0,0,0};
    D3DXMATRIX matrices[PablitoRigData::BoneCount];
    static float Clamp(float v,float low,float high){return (std::max)(low,(std::min)(high,v));}
    static float Smooth(float v){v=Clamp(v,0,1);return v*v*(3-2*v);}
    static D3DXVECTOR3 Mix(D3DXVECTOR3 a,D3DXVECTOR3 b,float t){return a+(b-a)*t;}
    static Pose Blend(const Pose& a,const Pose& b,float weight)
    {
        Pose p;
        p.body=Mix(a.body,b.body,weight);p.head=Mix(a.head,b.head,weight);p.bodyOffset=Mix(a.bodyOffset,b.bodyOffset,weight);
        p.leftWing=Mix(a.leftWing,b.leftWing,weight);p.rightWing=Mix(a.rightWing,b.rightWing,weight);
        p.leftReach=Mix(a.leftReach,b.leftReach,weight);p.rightReach=Mix(a.rightReach,b.rightReach,weight);
        p.leftFoot=Mix(a.leftFoot,b.leftFoot,weight);p.rightFoot=Mix(a.rightFoot,b.rightFoot,weight);
        p.leftStep=Mix(a.leftStep,b.leftStep,weight);p.rightStep=Mix(a.rightStep,b.rightStep,weight);
        p.scaleY=a.scaleY+(b.scaleY-a.scaleY)*weight;p.eyeOpen=a.eyeOpen+(b.eyeOpen-a.eyeOpen)*weight;p.mouth=a.mouth+(b.mouth-a.mouth)*weight;
        return p;
    }
    static D3DXMATRIX Joint(D3DXVECTOR3 pivot,D3DXVECTOR3 rotation,D3DXVECTOR3 offset=D3DXVECTOR3(0,0,0),D3DXVECTOR3 scale=D3DXVECTOR3(1,1,1))
    {
        D3DXMATRIX from,r,s,to;D3DXMatrixTranslation(&from,-pivot.x,-pivot.y,-pivot.z);
        D3DXMatrixRotationYawPitchRoll(&r,rotation.y,rotation.x,rotation.z);D3DXMatrixScaling(&s,scale.x,scale.y,scale.z);
        D3DXMatrixTranslation(&to,pivot.x+offset.x,pivot.y+offset.y,pivot.z+offset.z);return from*s*r*to;
    }
    void BuildMatrices()
    {
        matrices[0]=Joint({0,1.0f,0},pose.body,pose.bodyOffset,{1,pose.scaleY,1});
        matrices[1]=Joint({-.12f,1.9f,0},pose.head)*matrices[0];
        matrices[2]=Joint({-.14f,1.635f,-.32f},pose.leftWing,pose.leftReach)*matrices[0];
        matrices[3]=Joint({-.14f,1.635f,.32f},pose.rightWing,pose.rightReach)*matrices[0];
        // Feet remain grounded independently of breathing and the body's bob.
        matrices[4]=Joint({-.03f,.38f,-.42f},pose.leftFoot,pose.leftStep);
        matrices[5]=Joint({-.03f,.38f,.42f},pose.rightFoot,pose.rightStep);
        matrices[6]=Joint({.59f,2.35f,-.68f},{0,0,0},{0,0,0},{1,pose.eyeOpen,1})*matrices[1];
        matrices[7]=Joint({.59f,2.35f,.68f},{0,0,0},{0,0,0},{1,pose.eyeOpen,1})*matrices[1];
        matrices[8]=Joint({.55f,2.2f,0},{0,0,-pose.mouth})*matrices[1];
    }
    void Carry(Pose& p) const
    {p.leftWing={.10f,-1.52f,0};p.rightWing={-.10f,1.52f,0};p.leftReach.x=p.rightReach.x=.12f;p.head.z-=.06f;}
    Pose ActionPose(Motion motion,Pose p) const
    {
        const float wave=sinf(time*10),u=Progress(),reach=sinf(D3DX_PI*u);
        if(motion==Motion::Pickup || motion==Motion::Place || motion==Motion::Drop || motion==Motion::Deliver || motion==Motion::Bottle)
        {
            p.body.z-=.28f*reach;p.head.z-=.22f*reach;
            p.leftWing={.45f,-1.35f,0};p.rightWing={-.45f,1.35f,0};
            p.leftReach.x=p.rightReach.x=.25f*reach;
            if(motion==Motion::Deliver){p.head.z+=.12f*wave;p.mouth=.12f*reach;}
            if(motion==Motion::Bottle){p.rightWing.y+=.22f*sinf(time*16);p.leftWing.x+=.12f*wave;}
        }
        else if(motion==Motion::Grind || motion==Motion::Stir)
        {
            p.body.z=-.12f;p.head.z=-.16f;p.bodyOffset.y=.10f;p.bodyOffset.x=.12f;
            p.leftWing={.75f,-1.3f,0};p.rightWing={-.75f,1.3f,0};p.leftReach.x=p.rightReach.x=.45f;
            if(motion==Motion::Grind){p.rightWing.x+=.17f*wave;p.leftWing.x-=.10f*wave;p.bodyOffset.y+=.025f*wave;}
            else {p.rightWing.x+=.15f*sinf(time*6.5f);p.rightWing.y+=.24f*cosf(time*6.5f);p.leftWing={-.65f,-.4f,0};p.leftReach={0,0,0};}
        }
        else if(motion==Motion::Heat)
        {
            p.body.z=-.15f;p.head.z=-.20f;
            p.leftWing={-.35f+.27f*wave,-.7f,0};p.rightWing={.35f-.27f*wave,.7f,0};
        }
        else if(motion==Motion::Mount || motion==Motion::Dismount)
        {
            p.leftWing={-.1f,-.15f,0};p.rightWing={.1f,.15f,0};
            p.leftFoot.z=-.35f*reach;p.rightFoot.z=.25f*reach;p.leftStep.y=p.rightStep.y=.16f*reach;
            p.body.z=-.15f*reach;
        }
        else if(motion==Motion::Land){p.scaleY=.88f;p.bodyOffset.y=-.08f;p.leftWing.x=-.7f;p.rightWing.x=.7f;}
        else if(motion==Motion::Read){Carry(p);p.head.z=-.32f;p.body.z=-.06f;}
        else if(motion==Motion::Nope){p.head.y+=.28f*sinf(time*17);p.mouth=.05f;}
        return p;
    }
public:
    PablitoAnimator(){Reset();}
    void Reset()
    {pose=Pose();current=event=Motion::Idle;time=phase=eventAge=eventLength=0;wasGrounded=true;transitionFrom={0,0,0};BuildMatrices();}
    static float Duration(Motion action)
    {
        switch(action){case Motion::Mount:return .75f;case Motion::Dismount:return .65f;case Motion::Land:return .24f;
        case Motion::Bottle:return .95f;case Motion::Deliver:return .85f;case Motion::Read:return 1.0f;
        case Motion::Grind:case Motion::Stir:return .85f;case Motion::Heat:return .7f;case Motion::Nope:return .4f;default:return .6f;}
    }
    void Play(Motion action,D3DXVECTOR3 previousFeetOffset=D3DXVECTOR3(0,0,0))
    {event=action;eventAge=0;eventLength=Duration(action);transitionFrom=previousFeetOffset;}
    Motion Current() const{return current;}
    float Time() const{return time;}
    float Progress() const{return eventLength>0?Clamp(eventAge/eventLength,0,1):1;}
    const D3DXMATRIX* Matrices() const{return matrices;}
    D3DXVECTOR3 TransitionOffset() const
    {
        if(event!=Motion::Mount && event!=Motion::Dismount)return {0,0,0};
        const float u=Progress();return transitionFrom*(1-Smooth(u))+D3DXVECTOR3(0,.45f*sinf(u*D3DX_PI),0);
    }
    D3DXVECTOR3 Grip() const
    {D3DXVECTOR3 grip(1.15f,1.68f,0),out;D3DXVec3TransformCoord(&out,&grip,&matrices[0]);return out;}
    D3DXVECTOR3 ToolHand() const
    {D3DXVECTOR3 tip(-.12f,1.63f,1.40f),out;D3DXVec3TransformCoord(&out,&tip,&matrices[3]);return out;}
    void Update(float seconds,const Input& input)
    {
        const float dt=std::isfinite(seconds)?Clamp(seconds,0,.1f):0;time+=dt;
        if(event!=Motion::Idle){eventAge+=dt;if(eventAge+0.00001f>=eventLength){event=Motion::Idle;transitionFrom={0,0,0};}}
        if(!wasGrounded && input.grounded && !input.riding && event==Motion::Idle)Play(Motion::Land);
        wasGrounded=input.grounded;
        Pose target;target.scaleY=1+.007f*sinf(time*2.2f);target.head.y=Clamp(input.lookYaw,-.48f,.48f);target.head.z=Clamp(-input.lookPitch,-.18f,.18f);
        const float blink=fmodf(time,4.7f);target.eyeOpen=blink>4.45f?Clamp(fabsf(blink-4.57f)/.12f,.06f,1):1;
        current=Motion::Idle;
        if(input.riding)
        {
            current=Motion::Ride;target.body={Clamp(input.steering,-1,1)*.25f,0,-.15f};
            target.leftWing={-.45f,-.15f,0};target.rightWing={.45f,.15f,0};
            target.head.x=-target.body.x*.4f;target.leftFoot.z=-.15f;target.rightFoot.z=-.15f;
            target.bodyOffset.y=.025f*sinf(time*3.5f);
        }
        else if(!input.grounded)
        {
            current=input.verticalSpeed>0?Motion::Jump:Motion::Fall;
            target.leftWing={-.12f,-.15f,0};target.rightWing={.12f,.15f,0};target.body.z=-.08f;
            target.leftFoot.z=target.rightFoot.z=input.verticalSpeed>0?-.4f:.18f;
            target.leftStep.y=target.rightStep.y=input.verticalSpeed>0?.12f:0;
        }
        else if(input.speed>.15f)
        {
            const bool run=input.speed>10;current=run?Motion::Run:Motion::Walk;
            phase+=dt*(3+input.speed*.7f);const float step=sinf(phase),lift=run?.23f:.12f;
            target.body.z=run?-.18f:-.07f;target.body.x=.045f*step;target.bodyOffset.y=(run?.10f:.05f)*fabsf(cosf(phase));
            target.head.x=-target.body.x*.6f;
            target.leftFoot.z=step*(run?.65f:.4f);target.rightFoot.z=-target.leftFoot.z;
            target.leftStep={step*.17f,(std::max)(0.0f,step)*lift,0};target.rightStep={-step*.17f,(std::max)(0.0f,-step)*lift,0};
            target.leftWing.y=-.18f+.22f*step;target.rightWing.y=.18f+.22f*step;
        }
        if(input.carrying){Carry(target);if(current==Motion::Idle)current=Motion::Carry;}
        if(input.work!=Motion::Idle && input.speed<.5f && input.grounded && !input.riding)
        {current=input.work;target=ActionPose(current,target);}
        if(event!=Motion::Idle)
        {current=event;target=Blend(target,ActionPose(event,target),sinf(Progress()*D3DX_PI));}
        if(input.victory || input.defeat)
        {
            event=Motion::Idle;transitionFrom={0,0,0};target=Pose();current=input.victory?Motion::Victory:Motion::Defeat;
            if(input.victory)
            {
                const float flap=.4f+.2f*sinf(time*8);
                target.leftWing={flap,-.15f,0};target.rightWing={-flap,.15f,0};
                target.leftReach.z=-.12f;target.rightReach.z=.12f;
                target.bodyOffset.y=.14f*fabsf(sinf(time*5));target.head.z=.08f*sinf(time*5);target.mouth=.13f;
            }
            else {target.body.z=-.23f;target.head.z=-.42f;target.leftWing.x=-1.35f;target.rightWing.x=1.35f;target.eyeOpen=.55f;}
        }
        pose=Blend(pose,target,1-expf(-dt*16));BuildMatrices();
    }
};
#endif
