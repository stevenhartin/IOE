struct PS_INPUT
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

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

////////////////////////////////////////////////////////////////////////////////

Texture2D DiffuseTexture;
Texture2D BumpTexture;

#include "common/common_defs.h"
#include "common/common.h"
#include "common/common_shadows.h"

////////////////////////////////////////////////////////////////////////////////

static const float BumpConstant = 1.0f;

////////////////////////////////////////////////////////////////////////////////

float4 main(PS_INPUT input) : SV_TARGET0
{
    // Calculate the normal, including the information in the bump map
	float4 pixelNormal = normalize(input.Nrm);
	float4 bumpSample = BumpConstant * (BumpTexture.Sample(g_samPoint, input.UV) * 2.0f - 1.0f);
	float3 bumpNrm = normalize(((bumpSample.x * input.Tangent) + (bumpSample.y * input.Binormal) + (bumpSample.z * pixelNormal)).xyz);

    float3 vEye = View.xyz;
    float3 V = normalize(mul(vEye - input.WPos.xyz, (float3x3)WorldInvTranspose));
	float3 L = normalize(mul(LightPos.xyz - input.WPos.xyz, (float3x3)WorldInvTranspose));
	float3 H = normalize(V + L);
	
	float4 vTextureColour = DiffuseTexture.Sample(g_samPoint, input.UV) * input.Colour;
     
    const float fIntensity = Intensity;

	float DistSqToLight = dot(LightPos.xyz - input.WPos.xyz, LightPos.xyz - input.WPos.xyz);
	const float fRadius = 50.0f;

	float fModifier = Square(saturate(1 - Square(DistSqToLight * Square(1.0f / fRadius)))) / (DistSqToLight + 1);
	fModifier = saturate(fModifier * fIntensity);
    float LightModifier = GetShadowModifier(input.WPos.xyz) /* * fModifier*/;
    //float LightModifier = 1.0f;
     
	//float diffuseIntensity = saturate(dot(normalize(mul(L, (float3x3)WorldInvTranspose)), bumpNrm));
	float diffuseIntensity = saturate(dot(L, bumpNrm));
	                                        
	float specLighting = pow(saturate(dot(H, bumpNrm)), 60.0);
     
    return saturate(vTextureColour * diffuseIntensity * LightModifier + float4(specLighting, specLighting, specLighting, specLighting));
}