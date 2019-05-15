cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
}

struct VS_INPUT
{
    float4 Pos      : POSITION;
    float4 Nrm      : NORMAL;
    float4 Colour   : COLOR;
    float2 UV	    : TEXCOORD0;
    float4 Tangent  : TEXCOORD1;
    float4 Binormal : TEXCOORD2;
};

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
    float4 WPos		: POSITION;
    float4 WNrm		: NORMAL;
    float2 UV		: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
    Out.WNrm = input.Nrm;
    Out.UV = input.UV;
    Out.WPos = input.Pos;

    return Out;
};