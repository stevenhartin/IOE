cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
}

struct VS_INPUT
{
    float4 Pos      : POSITION;
    float3 UV	    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);

    return Out;
};