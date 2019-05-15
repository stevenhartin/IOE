struct VS_INPUT
{
    float4 Pos      : POSITION;
    float4 Nrm      : NORMAL;
    float4 Colour   : COLOR;
    float2 UV	    : TEXCOORD0;
    float4 Tangent  : TEXCOORD1;
    float4 Binormal : TEXCOORD2;
};

cbuffer GlobalSettings
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
}

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
    float4 WPos		: TEXCOORD2;
    float4 HPos		: POSITION;
    float2 UV		: TEXCOORD0;
    float4 Normal	: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
	Out.HPos = Out.Pos;
    //Out.Normal = float4(mul(input.Nrm.xyz, (float3x3)WorldInvTranspose), 0.0f);
    Out.Normal = float4(input.Nrm.xyz, 0.0f);
    Out.UV = input.UV;
    Out.WPos = input.Pos;

    return Out;
}
