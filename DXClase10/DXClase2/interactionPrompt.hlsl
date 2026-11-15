cbuffer PromptDraw : register(b0)
{
    float4 AnchorClip;
    float4 SizeAndOffset;
};

struct PROMPT_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PROMPT_OUTPUT VS(uint vertexId : SV_VertexID)
{
    PROMPT_OUTPUT output;
    output.uv = float2(vertexId & 1, vertexId >> 1);
    output.position = AnchorClip;
    float2 offset = float2(output.uv.x - 0.5f, 0.5f - output.uv.y) * SizeAndOffset.xy;
    output.position.xy += (offset + SizeAndOffset.zw) * AnchorClip.w;
    return output;
}

Texture2D KeyIcon : register(t0);
SamplerState IconSampler : register(s0);

float4 PS(PROMPT_OUTPUT input) : SV_Target
{
    // Preserve the supplied image and its soft transparent edges, independent of light.
    return KeyIcon.Sample(IconSampler, input.uv);
}
