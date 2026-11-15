cbuffer ShadowDraw : register(b0)
{
    float4x4 WorldLightViewProjection;
};
struct SHADOW_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXTURE;
};
SHADOW_OUTPUT VS(float3 position : POSITION, float2 uv : TEXTURE)
{
    SHADOW_OUTPUT output;
    output.position = mul(float4(position, 1), WorldLightViewProjection);
    output.uv = uv;
    return output;
}
Texture2D Albedo : register(t0);
SamplerState MaterialSampler : register(s0);
void PS(SHADOW_OUTPUT input)
{
    // Transparent cutouts cast their silhouette instead of a solid rectangle.
    clip(Albedo.Sample(MaterialSampler, input.uv).a - 0.9f);
}
