#include "common/common_defs.h"

cbuffer Globals
{
    float4x4 WorldViewProj : WORLDVIEWPROJ;
}

struct VS_INPUT
{
    float4 Pos      : POSITION;
    float4 Colour   : COLOR;
    float2 UV	    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos		: SV_POSITION;
    float4 Colour	: COLOR;
    float2 UV		: TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.Pos = mul(float4(input.Pos.xyz, 1.0f), WorldViewProj);
    Out.Colour = input.Colour;
    Out.UV = input.UV;
    
    return Out;
};