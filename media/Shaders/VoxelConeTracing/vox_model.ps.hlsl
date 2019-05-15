////////////////////////////////////////////////////////////////////////////////

cbuffer RarelyUsed
{
    float OctreeSize;
    float Near;
    float Far;
}

////////////////////////////////////////////////////////////////////////////////

cbuffer GlobalSettings
{
    float4x4 WorldToOctree : WORLDTOOCTREE;
    float4x4 WorldViewProj : WORLDVIEWPROJ;
    float4x4 WorldInvTranspose : WORLDINVTRANSPOSE;
};

////////////////////////////////////////////////////////////////////////////////

cbuffer LightSettings
{
    float4 LightPos : LIGHTPOS;
    float4 LightColour : LIGHTCOLOUR;
    float Intensity : INTENSITY;
    float Roughness : ROUGHNESS;
	float LightSpecular : LIGHTSPECULAR;
	float LightRadius : LIGHTRADIUS;
};

////////////////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
    float4 Pos		: SV_POSITION;
    float4 WPos		: TEXCOORD2;
    float4 HPos		: POSITION;
    float2 UV		: TEXCOORD0;
    float4 Normal	: TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

////////////////////////////////////////////////////////////////////////////////

Texture2D DiffuseTexture;
RWTexture3D<float4> Voxels;

////////////////////////////////////////////////////////////////////////////////

static const int NUM_SAMPLES_WIDTH = 1;
static const int NUM_SAMPLES_HEIGHT = 1;

////////////////////////////////////////////////////////////////////////////////

#include "../common/common_defs.h"
#include "../common/common_shadows.h"

////////////////////////////////////////////////////////////////////////////////

float Square(float x)
{
	return x * x;
}

////////////////////////////////////////////////////////////////////////////////

float4 main(PS_INPUT input) : SV_TARGET0
{
	float2 fChangeX = ddx(input.UV) / NUM_SAMPLES_WIDTH;
	float2 fChangeY= ddy(input.UV) / NUM_SAMPLES_HEIGHT;

	float4 DiffuseTex = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[unroll]
	// Sub-cell bluring, average out the cell's ligting
	for (int i=-NUM_SAMPLES_HEIGHT/2+(NUM_SAMPLES_HEIGHT%2==0?1:0); i <= NUM_SAMPLES_HEIGHT/2; ++i)
	{
		float2 OffsetY = fChangeY * i;
		[unroll]
		for (int j=-NUM_SAMPLES_WIDTH/2+(NUM_SAMPLES_WIDTH%2==0?1:0); j <= NUM_SAMPLES_WIDTH/2; ++j)
		{
			float2 OffsetX = fChangeX * j;
			DiffuseTex += DiffuseTexture.SampleLevel(g_samPoint, input.UV + OffsetX + OffsetY, 0);
		}
	}

	DiffuseTex /= NUM_SAMPLES_WIDTH * NUM_SAMPLES_HEIGHT;

	// Transparent objects don't reflect light, ignore any with <50pct alpha
	clip(DiffuseTex.a - 0.5f);

	float3 BumpNormal = normalize(input.Normal.xyz); 

	// calculate direct lighting
	float Shadowing = GetShadowModifier(input.WPos.xyz);
	
	float3 L = normalize(mul(LightPos.xyz - input.WPos.xyz, (float3x3)WorldInvTranspose));
	float LightingNL = clamp(dot(BumpNormal, L), 0.0, 1.0);
	
	float DistSqToLight = dot(LightPos.xyz - input.WPos.xyz,
		LightPos.xyz - input.WPos.xyz);
	const float Radius = LightRadius;
	
	float LightRadius = Square(saturate(1.0f - Square(DistSqToLight * Square(1.0f / Radius)))) / (DistSqToLight + 1.0f);
	LightRadius = saturate(LightRadius * Intensity);
	
	float3 DiffuseLight = LightColour.xyz * (LightingNL * Shadowing) * LightRadius;

	float3 Diffuse = DiffuseTex.xyz * DiffuseLight;

	float4 OutColor = float4(Diffuse, 1.0);

	// store voxel	
	float4 StoragePos = mul(float4(input.HPos.xyz, 1.0f), WorldToOctree);
	StoragePos.xyz *= 1.0 / StoragePos.w;
	
	uint3 VoxelPos = uint3(((StoragePos.xyz * 0.5f) + float3(0.5f, 0.5f, 0.5f)) * OctreeSize);

	// compress outColor range to encode limited HDR
	OutColor.xyz *= 1.0f - Roughness;
	Voxels[VoxelPos] = (OutColor);

	return OutColor;
}

////////////////////////////////////////////////////////////////////////////////