#ifndef GC_SUN_LIGHTING_HLSLI
#define GC_SUN_LIGHTING_HLSLI

struct Light
{
    float3 direction;
    float specularPower;
    float4 ambient;
    float4 diffuse;
};
cbuffer ConstantBufferPS : register(b0) { Light light; };

// Matches SunLight::Data. t0-t2 and s0 remain the mesh's material slots.
cbuffer SunBuffer : register(b2)
{
    float4x4 SunViewProjection;
    float4 SunDirection;
    float4 SunColor;
    float4 SunShadow; // texel size, receiver bias, normal offset, strength
    float4 SunSky;    // inner/outer disk cosine, halo exponent, halo strength
    float4 MoonDirection; // direction, visibility
    float4 MoonRight;     // billboard right, tangent of angular radius
    float4 MoonUp;        // billboard up, night painting exposure
    float4 NightSky;      // daylight, stars, twilight, star rotation
    float4 RoomOrigin;
    float4 RoomAxes;      // inverse horizontal basis of the main house
    float4 RoomLight;     // soft, fixed interior ambient contribution
};
Texture2D<float> SunShadowMap : register(t3);
SamplerComparisonState SunShadowSampler : register(s1);

float SunVisibility(float3 worldPosition, float3 normal)
{
    float facing = saturate(dot(normal, light.direction));
    float3 receiver = worldPosition + normal * SunShadow.z * (1.0f - facing);
    float4 projected = mul(float4(receiver, 1), SunViewProjection);
    float3 lightPosition = projected.xyz / projected.w;
    float edge = max(abs(lightPosition.x), abs(lightPosition.y));
    if (edge >= 1.0f || lightPosition.z <= 0 || lightPosition.z >= 1) return 1;
    float2 uv = lightPosition.xy * float2(0.5f, -0.5f) + 0.5f;
    float visibility = 0;
    [unroll] for (int y = -1; y <= 1; ++y)
    [unroll] for (int x = -1; x <= 1; ++x)
        visibility += SunShadowMap.SampleCmpLevelZero(SunShadowSampler,
            uv + float2(x, y) * SunShadow.x, lightPosition.z - SunShadow.y);
    visibility /= 9.0f;
    // Fade out at the map's perimeter rather than showing a square boundary.
    visibility = lerp(visibility, 1, smoothstep(0.85f, 0.98f, edge));
    return lerp(1, visibility, SunShadow.w);
}

float3 SunLitColor(float3 albedo, float3 normal, float3 worldPosition)
{
    normal = normalize(normal);
    float diffuse = saturate(dot(normal, light.direction));
    float3 relative=worldPosition-RoomOrigin.xyz;
    float x=dot(relative.xz,RoomAxes.xy),z=dot(relative.xz,RoomAxes.zw);
    float room=smoothstep(-11,-9,x)*(1-smoothstep(22,24,x));
    room*=smoothstep(-10,-8,z)*(1-smoothstep(9,11,z));
    room*=smoothstep(-.25f,.2f,relative.y)*(1-smoothstep(7,8,relative.y));
    return albedo * (light.ambient.rgb + room*RoomLight.rgb + light.diffuse.rgb * diffuse * SunVisibility(worldPosition, normal));
}

#endif
