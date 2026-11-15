cbuffer ConstantBuffer
{
	float4x4 WorldViewProjection;
	float4x4 World;
	float3 CameraPos;
	float MaxH;
	float4 TerrainMapping;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXTURE;
	float3 normal : NORMAL;
	float3 viewDirection : COORDS;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 vectPos : POSITION;
	float maxHeight : HEIGHT;
	float3 worldPosition : TEXCOORD1;
	float2 terrainUV : TEXCOORD2;
	float cameraDistance : TEXCOORD3;
};

VS_OUTPUT VS(float4 inPos : POSITION, float4 inTextCoord : TEXTURE, float3 normal : NORMAL, float3 tangent : TANGENT, float3 binormal : BINORMAL)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, WorldViewProjection);
	output.normal = mul(float4(normal, 0), World).xyz;
    output.TexCoord = inTextCoord;
	output.worldPosition = mul(inPos, World).xyz;
	output.viewDirection = normalize(CameraPos.xyz - output.worldPosition);
	output.cameraDistance = length(CameraPos.xyz - output.worldPosition);
	output.tangent = mul(float4(tangent, 0), World).xyz;
	output.binormal = mul(float4(binormal, 0), World).xyz;
	output.vectPos = inPos;
	output.maxHeight = MaxH;
	output.terrainUV = inPos.xz * TerrainMapping.xy + float2(0, 1);

    return output;
}
