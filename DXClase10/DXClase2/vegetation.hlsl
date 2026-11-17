#include "SunLighting.hlsli"

cbuffer VegetationView : register(b1)
{
    float4x4 ViewProjection;
    float4 CameraRight;
    float4 CameraPosition;
    float4 TextureBounds;
};
struct Instance
{
    float3 position : POSITION;
    float2 size : SIZE;
    float3 tint : COLOR;
    float mirror : TEXCOORD0;
    float3 normal : NORMAL;
    float2 slope : TEXCOORD1;
};
struct Pixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 world : TEXCOORD1;
    float3 tint : COLOR;
    float3 normal : NORMAL;
    float distance : TEXCOORD2;
};
Pixel VS(Instance plant,uint id : SV_VertexID)
{
    const float2 corners[6]={float2(0,1),float2(0,0),float2(1,0),float2(0,1),float2(1,0),float2(1,1)};
    float2 corner=corners[id];
    float side=(corner.x-0.5f)*plant.size.x;
    float3 world=plant.position+CameraRight.xyz*side;
    // Follow the slope along the billboard's base instead of floating above it.
    world.y+=dot(plant.slope,CameraRight.xz)*side+(1-corner.y)*plant.size.y;
    Pixel output;
    output.position=mul(float4(world,1),ViewProjection);
    float u=plant.mirror>0?1-corner.x:corner.x;
    output.uv=lerp(TextureBounds.xy,TextureBounds.zw,float2(u,corner.y));
    output.world=world;output.tint=plant.tint;output.normal=plant.normal;
    output.distance=distance(CameraPosition.xyz,world);
    return output;
}
Texture2D PlantTexture : register(t0);
SamplerState PlantSampler : register(s0);
float4 PS(Pixel input) : SV_Target
{
    float4 plant=PlantTexture.Sample(PlantSampler,input.uv);
    clip(plant.a-0.40f);
    // Screen-door distance fade keeps proper depth occlusion without sorting.
    const float thresholds[16]={0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5};
    uint2 pixel=uint2(input.position.xy)%4;
    float fade=1-smoothstep(260.0f,350.0f,input.distance);
    clip(fade-(thresholds[pixel.y*4+pixel.x]+0.5f)/16.0f);
    float3 color=SunLitColor(plant.rgb*input.tint,input.normal,input.world);
    float haze=1-exp(-max(input.distance-160.0f,0.0f)*0.00065f);
    float3 hazeColor=lerp(float3(0.085f,0.115f,0.17f),float3(0.59f,0.71f,0.76f),NightSky.x);
    return float4(lerp(color,hazeColor,haze),1);
}
