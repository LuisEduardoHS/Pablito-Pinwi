#include "SunLighting.hlsli"
cbuffer WaterView : register(b1)
{
    float4x4 ViewProjection;
    float4 CameraPosition;
    float4 WaterTime; // seconds, wave amplitude, inverse noise texture size, padding
};
struct Pixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 world : TEXCOORD1;
    float3 normal : NORMAL;
};
Pixel VS(float3 position : POSITION,float2 uv : TEXCOORD0,float3 normal : NORMAL)
{
    float shore=sin(saturate(uv.x)*3.14159265f);
    position.y+=shore*WaterTime.y*(sin(uv.y*5.0f-WaterTime.x*2.2f)+.5f*cos(uv.x*8+uv.y*2-WaterTime.x*1.4f));
    Pixel output;output.world=position;output.position=mul(float4(position,1),ViewProjection);output.uv=uv;output.normal=normal;return output;
}
Texture2D FlowTexture : register(t0);
SamplerState FlowSampler : register(s0);
float4 PS(Pixel input) : SV_Target
{
    float2 flowA=float2(input.uv.x*1.6f,input.uv.y*.7f-WaterTime.x*.16f);
    float2 flowB=float2(input.uv.x*2.7f+WaterTime.x*.035f,input.uv.y*.43f-WaterTime.x*.10f);
    float a=FlowTexture.Sample(FlowSampler,flowA).r,b=FlowTexture.Sample(FlowSampler,flowB).g;
    float step=WaterTime.z;
    float dx=FlowTexture.Sample(FlowSampler,flowA+float2(step,0)).r-FlowTexture.Sample(FlowSampler,flowA-float2(step,0)).r;
    float dz=FlowTexture.Sample(FlowSampler,flowA+float2(0,step)).r-FlowTexture.Sample(FlowSampler,flowA-float2(0,step)).r;
    float3 normal=normalize(input.normal+float3(-dx*2.0f,0,-dz*2.0f));
    float3 view=normalize(CameraPosition.xyz-input.world);
    float fresnel=pow(1-saturate(dot(normal,view)),4);
    float edge=min(input.uv.x,1-input.uv.x);
    float shore=1-smoothstep(.02f,.15f,edge);
    float streak=pow(saturate(a*.65f+b*.35f),5);
    float3 albedo=lerp(float3(.025f,.28f,.37f),float3(.10f,.60f,.68f),.25f+.6f*a);
    float3 color=SunLitColor(albedo,normal,input.world);
    float3 sky=lerp(float3(.08f,.14f,.25f),float3(.48f,.71f,.87f),NightSky.x);
    color=lerp(color,sky,.18f+.32f*fresnel);
    float specular=pow(saturate(dot(reflect(-light.direction,normal),view)),64);
    color+=light.diffuse.rgb*specular*.65f;
    float foam=saturate(streak*1.5f+shore*(.28f+.32f*b));
    color=lerp(color,float3(.71f,.91f,.91f)*(.45f+.55f*NightSky.x),foam);
    float alpha=(.80f+.16f*fresnel)*smoothstep(0,.025f,edge);
    alpha*=smoothstep(0,.25f,input.uv.y)*smoothstep(0,.25f,12-input.uv.y);
    return float4(color,alpha);
}
