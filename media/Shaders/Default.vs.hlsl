#include "common/common_defs.h"

cbuffer GlobalSettings
{
    float4 View : VIEW;
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
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
	float4 WPos		: POSITION;
    float4 Pos		: SV_POSITION;
    float4 Nrm		: NORMAL;
	float4 WNrm		: TEXCOORD0;
    float4 Colour	: COLOR;
    float2 UV		: TEXCOORD1;
    float4 Tangent	: TEXCOORD2;
    float4 Binormal	: TEXCOORD3;
};

////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
    Out.Colour = input.Colour;
	Out.WNrm = input.Nrm;
    Out.Nrm = float4(mul(input.Nrm.xyz, (float3x3)WorldInvTranspose), 0.0f);
	Out.Tangent = float4(mul(input.Tangent.xyz, (float3x3)WorldInvTranspose), 0.0f);
	Out.Binormal = float4(mul(input.Binormal.xyz, (float3x3)WorldInvTranspose), 0.0f);
    Out.UV = input.UV;
	Out.WPos = input.Pos;
    
    return Out;
};